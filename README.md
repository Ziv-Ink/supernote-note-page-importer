# Supernote Note Page Importer

A Supernote plugin with a popup that replaces the currently open note page’s
TOTALPATH with page one from a selected `.note` file. It uses the bundled
SNfiletools C++ library through a generated local module, and Supernote’s
`sn-plugin-lib` APIs to save, reload, regenerate and display the page.

Open a destination note and page, launch **file_test_2** from the Notes plugin
menu, choose a different source note, then press **Replace page**. The popup
shows the destination filename and page number. Cancel leaves the note unchanged.
A blank first source page clears the destination strokes; later source pages are
never substituted for page one.

The import replaces TOTALPATH records, including any non-stroke elements stored
in that block. It retains the destination page’s template and other metadata.
The destination must remain open on the captured page. The source is not modified.

## Build and run

Requirements: Node.js 18+, JDK 17, an Android SDK with platform 35, the NDK/CMake
versions required by `android/build.gradle` and `android/gradle.properties`, and
an ADB-connected Supernote with PluginHost installed. The tested device is a
Supernote A5 X2 (SN100C). The cache refresh bridge depends on its firmware’s
Notes Binder interface.

```sh
npm ci
export JAVA_HOME=/path/to/jdk-17
export ANDROID_HOME=/path/to/Android/Sdk
npm run build
npm run verify
npm run deploy
npm run run
```

Alternatively copy `devconfig.example.json` to `devconfig.json` and set the three
local paths. Linux scripts also require `jq`, `adb`, and normal zip utilities.
`npm run deploy` builds, verifies and installs the package; `npm run run` launches
it through the Notes plugin menu. Progress and errors use `console.log` and
`console.error`, visible under the `ReactNativeJS` logcat tag.

Allow the requested file READ and WRITE permissions. Permission handling lives
in `src/permissions.ts`.

## Backups and refresh

Each import keeps a verified whole-note backup and intermediate snapshots under
`/storage/emulated/0/Note/snfiletools-page-backups/`. These remain on the device;
copy the original `replace-page-*.note` backup to recover a prior whole note.
The refresh clears Notes’ cache, reloads the edited page, then asks Notes to
delete a checked-absent element number to trigger redraw. Saving waits for that
redraw. The importer restores the exact source TOTALPATH bytes while retaining
the regenerated bitmap, publishes a new inode at the same path, then opens the
same destination page. This refresh clears Notes’ undo/redo history.

No fixed sleep is used in the import workflow. Final `openFile` returns before
Notes finishes displaying the page, so logged elapsed time excludes that final
load and physical e-ink refresh.

## Development and tests

```sh
npm test -- --runInBand
npx eslint App.tsx src/note_context.ts src/insert_totalpath.ts src/permissions.ts __tests__/page_import.test.ts
npx tsc --noEmit --jsx react-native --esModuleInterop --skipLibCheck --moduleResolution node --target es2020 App.tsx src/insert_totalpath.ts src/note_context.ts src/permissions.ts
```

Native tests require local note/PDF-mark fixtures; personal fixture files are
not included in this repository:

```sh
cmake -S local_modules/snfiletools/tests -B build/snfiletools-tests \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DSNFILETOOLS_TEST_FILES_DIR=/path/to/test_files
cmake --build build/snfiletools-tests
ctest --test-dir build/snfiletools-tests --output-on-failure
```

The native suite verifies byte-exact payload copies, source immutability,
backups, sibling-page preservation, invalid-page rejection, and a blank source
page one followed by a nonempty page two. JavaScript tests cover captured page
targeting, empty imports, changed destinations and redraw ordering.

Generated bindings and the matching runtime are committed, so ordinary builds
do not need the module generator. For native API changes, use the matching
Supernote module generator **0.1.3** and the wrapper documented in
`local_modules/snfiletools/README.md`. Newer generator formats require migration.

Device validation on 2026-10-01: selecting a source through the native picker
replaced destination page two with source page one (two strokes and one text
box), regenerated its bitmap and displayed it without a lasso operation. The
edit logged 1,290 ms before final page loading. A blank first source page and
popup cancellation were also exercised. The clean repository passed `npm ci`,
`npm test`, `npm run build`, and `npm run verify`; the workspace was installed
and launched with `npm run deploy` and `npm run run`.
