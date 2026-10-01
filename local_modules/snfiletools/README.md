# SNfiletools local module

The `SNfiletools/` directory contains a source snapshot from
the author’s `read_file_on_device_cpp/SNfiletools` library, at commit
`ff18eb09794fb0b7ff4e29fb4f512929706ddfa1`. It is included in the npm package and
compiled into the Supernote runtime through this module's `CMakeLists.txt`.
Changes in the original folder must be copied here explicitly to update the snapshot.

```ts
import Snfiletools from 'snfiletools';

const bytes: bigint = await Snfiletools.getSize('/storage/emulated/0/Note/example.note');
console.log(bytes.toString());
```

`getSize(path)` accepts `.note`, `.pdf`, and `.pdf.mark` paths. PDF inputs require
both the PDF and its `.pdf.mark` companion; the returned size is the mark file's
size. Calls return Promises and run on the generated worker executor. Library
errors reject those Promises.

Shared-storage reads, including direct C++ filesystem access, require
`plugin.permission.FILE:READ` in the plugin's `PluginConfig.json`
`uses-permissions` array. Check `PluginManager.hasPermission()` and request it
with `PluginManager.requestPermission()` before calling `getSize()`.
The plugin's private directory is accessible without this permission.

`await setJournalDirectory(path)` configures persistent app-private journal storage
before future write operations. Supply a directory accessible to the plugin host.
The bridge serializes all library operations with one mutex. Existing
`greet` and `greetFromJvm` calls are still available. Generated API details are in
`.supernote-generated/README.md` and `.supernote-generated/index.d.ts`.

After editing marked native declarations, regenerate the bindings from the plugin
root, then build the Android plugin:

```sh
JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64 \
SUPERNOTE_GRADLE_COMMAND="$PWD/scripts/moduleGradle.sh" \
sn-module-gen update --yes
```

The wrapper adds the Android platform jar to the generator's standalone Kotlin
analysis classpath so it can resolve `Context` and generate constructor injection.
It reads the SDK location from `ANDROID_HOME` or `devconfig.json`. The normal
Android module build already supplies these types.

## Native bridge tests

Use the real fixtures supplied by the original library without modifying them:

```sh
cmake -G Ninja -S local_modules/snfiletools/tests -B build/snfiletools-tests \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DSNFILETOOLS_TEST_FILES_DIR=/home/ziv/Desktop/supernote/tools/read_file_on_device_cpp/tests/test_files
cmake --build build/snfiletools-tests
ctest --test-dir build/snfiletools-tests --output-on-failure
```

The vendored doctest tests cover every note and PDF/mark pair, invalid paths,
journal configuration, concurrent reads that exceed the library's cache size,
metadata edits, TOTALPATH edits, and main-layer bitmap replacements with byte-exact
restoration on copied fixtures. Bitmap tests also verify snapshot publication
preserves every edited byte and refuses changed or identical source paths.
Paste tests import each nonblank supplied note into a blank page and then into
the populated page, verify merged trail framing/read-back, preserve the source,
and restore the destination backup.
Clang matches the Android toolchain; GCC 14 rejects the upstream portable I/O
header's `file_state` struct/function naming under its `-Werror=shadow` policy.

## Earlier open-note bitmap diagnostic

The retained `src/test_open_note.ts` bitmap diagnostic saves the currently open throwaway note, requests
`plugin.permission.FILE:WRITE`, and calls
`testBitmapReplace(path, patternPath, backupPath, pageKey)`. Both the destination
path and current page come from `PluginCommAPI`. The native helper follows the
page's MAINLAYER pointer and that layer's LAYERBITMAP pointer; it replaces only
the bitmap payload through the C++ library's block writer. The supplied RLE
chart changes a blank 600-byte bitmap to 94,324 bytes.

The diagnostic compares SDK reload with the writer cached, SDK reload after
releasing the cached writer, SDK save/reload, and SDK `openFile`. It then
publishes the verified edited snapshot at the same path with a fresh inode,
reopens with `PluginFileAPI.openFile(path, page)`, and tests SDK save/reload
on a subsequent invocation. `openFile` hides/pauses the plugin, so reopening
must be the final action of the first invocation. This additional step tests Notes' inode-based working cache; publishing
is refused if the current bytes differ from the snapshot. Keep the note idle
throughout this diagnostic. SDK success responses alone do not establish that
the native page loader or rendering succeeded; inspect the screen and native
Notes logs as well.

The test helpers live in `tests/device_edit.cpp`; the vendored library's public
API and sources are unchanged. Complete backups are necessary because a block
write can compact old save history. Original and edited snapshots are retained
under `/storage/emulated/0/Note/snfiletools-test-backups/`. Errors restore the
original bytes. Successful experiments retain the bitmap. The chart asset and
its provenance are in `tests/fixtures/`. Upload it to the expected device path
before running this diagnostic. Results use `console.log` and are available
from ReactNativeJS logcat after `npm run run`.

The earlier TOTALPATH paste helper remains available for research. Before the
cache/reopen and lasso steps were identified, its test copied
raw trails without remapping their page/layer/trail identities or updating
bitmap previews. Its successful byte checks and SDK acknowledgments did not
qualify native rendering: later reloads stalled with loader error 17. The
pre-paste backup restored the note. Do not infer rendered stroke-import support
from that helper.

### Device result, October 1, 2026

The connected Nomad's current note, `/storage/emulated/0/Note/20260929_235038.note`,
passed bitmap replacement and visual verification. Plain SDK reload, closing
cached C++ streams, and opening with the existing inode left the display blank
or loading; native Notes reported `preLoadNotePage` error 17 despite successful
SDK responses. A process restart with the same inode did not resolve this.
Publishing the identical edited bytes with a new inode at the same path and
calling SDK `openFile` displayed the exact squares, checkerboard, and bars.
The plugin now performs that publication/reopen step itself. Notes retained
PID 26256 throughout the final plugin-driven edit/reopen test.

On the next `npm run run`, SDK save/reload succeeded, the native loader returned
0 and completed loading, the screen still displayed the pattern, and the note's
on-device SHA-256 still matched the edited snapshot:
`7a0d4d3148b9826f9edbfb765d66506e81f62d966632153ba378221b319ee741`.
Native save reported no dirty changes (`isSave false`); this test establishes
that the SDK calls preserve an already rendered bitmap, not serialization after
new handwriting. Bitmap-only replacement does not create editable vector strokes.

The original backup is
`/storage/emulated/0/Note/snfiletools-test-backups/bitmap-1790856594037.note`, with
SHA-256 `ef6dcfc0ca3382a35aa6e09dcd9740dc3d1d402e902c17407ddfc954182adb76`.
Its `.edited.note` companion contains the verified replacement. Device console,
native logs, screenshots and hash evidence are retained in the plugin root's
`build/snfiletools-bitmap-*` files; the final screenshot is
`build/snfiletools-bitmap-after-save.png`.

The native suite passed 8 cases / 856 assertions. Generated-module validation,
package verification, targeted ESLint/TypeScript checks and the subsequent
`npm run run` passed. The final deploy built and installed successfully; its
launch-acknowledgment check timed out while waiting at the permission dialog.
Actual plugin execution was observed in ReactNativeJS and the later run exited 0.
The current host writer also reproduced the older research experiment's edited
fixture byte-for-byte, with SHA-256
`0f1824755a1a2ceaa5d4434b87e7284819fe9d632ec9311567cc450dff2218a7`.

## Retained raw TOTALPATH and lasso diagnostic

The earlier entry point was `src/test_totalpath_refresh.ts`. To use it, select
that diagnostic in `App.tsx`, run `npm run deploy`, then
`npm run run` twice, granting WRITE for each invocation if prompted. The first
invocation saves the SDK's current note/page, resets the library's private
journal configuration to release cached readers, and calls `testTotalPathCopy`
with `/storage/emulated/0/Note/test.note` as source. The helper copies the first
nonempty TOTALPATH byte for byte, replacing the destination payload (without
remapping records), verifies library and disk read-back, and leaves the original
bitmap unchanged. It retains complete original/edited snapshots on-device.
It publishes identical bytes at a new inode and calls SDK `openFile` last.

The second invocation resumes from plugin-private state stored through Kotlin
exports. It calls `PluginCommAPI.lassoElements` over the full page, then
`setLassoBoxState(2)` to finish selection, `PluginNoteAPI.saveCurrentNote`, and
`PluginCommAPI.reloadFile`. `testBitmapUnchanged` compares MAINLAYER bitmap bytes
against the original backup before lasso, after save, and after reload. Subsequent
invocations verify the completed state without importing again. The full-page
rectangle is currently 1920 x 2560, matching the tested device; other dimensions,
orientations, source layers, and unrelated notes have not been qualified.

### Verified raw-copy result, October 1, 2026

The initial destination failed library indexing with `Block length outside file`.
Its canonical bytes were preserved on-device at
`/storage/emulated/0/Note/snfiletools-test-backups/before-totalpath-recovery-20261001.note`.
The previously verified throwaway-note snapshot was restored with a new inode and
reopened using Notes' VIEW intent. Native loading returned 0. No private Notes
cache files were accessed or deleted.

The plugin then copied 19,447 exact source payload bytes, containing 3 records.
The destination stayed visually blank until SDK lasso. Notes selected two trails
and one normal text box, displayed both strokes and the text "testing", and
regenerated the bitmap when selection finished. Native save reported `isSave true`.
The changed bitmap survived SDK reload (`preLoadNotePage` return 0) and a further
plugin save/reload invocation. Notes retained PID 26256. No direct invocation of
Notes' Java/JNI renderer was necessary; the public SDK lasso reaches that native
redraw path. The Kotlin exports were exercised for private diagnostic state.

The verified pre-copy snapshot is
`/storage/emulated/0/Note/snfiletools-test-backups/totalpath-refresh-1790861875509.note`;
its `.edited.note` companion retains the exact raw-copy state before lasso.
The source SHA-256 remained
`8cdfbb60fe6d2d5fc60823ee1e09b997ccbe95163b270cd95e9271f5f0198452`.
Logs, hashes, screenshots, and the structured result are in `build/snfiletools-totalpath-*`.
The native bridge suite now passes 9 cases, including exact replacement into
already populated destination pages and preservation of bitmap/source/backup.

After replacing native binaries, a PluginHost restart was needed to avoid stale
loaded libraries and exception messages. This restarted only PluginHost, without
clearing its data. Reopening the current note through Notes' VIEW intent restored
the SDK's current-file state. The button callback now has one rejection handler
because the SDK listener does not await asynchronous callbacks; it logs test
failures to `console.error`.

## Retained identity-update redraw diagnostic

The earlier entry point was `src/test_direct_redraw.ts`. This research diagnostic requires
the same prepared throwaway note/page and private state from the raw-copy test.
It refuses a different current note/page. Run `npm run deploy:run`, then
`npm run run` for a persistence check. WRITE permission is required.

The inspected Notes APK exposes VIEW/open, plugin binder methods, providers,
and launcher broadcast handling. No dedicated rebuild-only command was found.
VIEW/open and SDK reload load existing bitmaps. The exported NoteService binder
only offers sticker migration; the providers expose file records/state, not a
renderer. The custom command handler supports navigation, links, and lasso points.
These unrelated endpoints were not mutated.

`com.ratta.supernote.launcher.flashscreen` reaches `LauncherBroadcast`, then
`NotePresenter.getFlashScreen()` and `forcedToLoadLayer()`. Android rejects this
protected action from the ADB shell UID. The local module's Context-injected
Kotlin `NoteRefresh.requestNoteRedraw()` successfully sent it through PluginHost's
normal `Context.sendBroadcast` from UID 1000. Native logs confirmed delivery,
but the bitmap stayed unchanged and save reported `isSave false`.

The working route is `PluginCommAPI.reloadFile()`, then current-page
`PluginFileAPI.getElements()`, followed by `PluginCommAPI.modifyPageElements()`
with the existing stroke objects, `PluginNoteAPI.saveCurrentNote()`, and reload.
The APK handler updates native trail cache and calls `onRedraw(true)`, which
forces layer loading. This resubmits stroke records and can normalize their
metadata; it does not promise byte-identical TOTALPATH after save. No lasso
selection is created. The native handler closes any existing selection.

### Verified direct result, October 1, 2026

The verified raw-copy snapshot was published at the current path with a fresh
inode and reopened before the experiment, restoring a blank bitmap with three
imported objects. The broadcast/save test left it blank. SDK enumeration returned
two strokes and one legacy text box. Updating all three was rejected by the SDK
validator because the text box reports `fontSize=0`; no native update occurred.
The diagnostic therefore preserves that text box and resubmits the two strokes.

`modifyPageElements` returned `[1,2]`, native logs showed `onRedraw` and a dirty
save (`isSave true`), and reload returned native loader code 0. Both strokes and
the text appeared. The RGB canvas pixels matched the earlier lasso rendering
exactly. A second plugin invocation saved/reloaded without another modification;
the regenerated bitmap and visible page persisted. Notes kept PID 26256.
This qualifies the prepared single-page fixture, not arbitrary note/layer types.

The source note hash remained
`8cdfbb60fe6d2d5fc60823ee1e09b997ccbe95163b270cd95e9271f5f0198452`.
The resulting destination hash is
`54fc554c2b6d73665ead27af4fcbb0ac74d3fdc4ed601b200bdf1ae2717f4e3c`.
Complete note snapshots remain on-device. Research logs, scalar hashes,
screenshots, and the result JSON are in `build/snfiletools-direct-redraw-*`.

`npm run deploy:run`, `npm run verify`, and the subsequent `npm run run` passed.
Generator validation through the wrapper and targeted TypeScript/ESLint checks
passed. The unchanged native bridge's nine-case suite passed in the preceding
raw-copy test. Full-project TypeScript checking still has unrelated errors in
`src/current_note_info.ts`.

## Active raw-byte-preserving redraw diagnostic

`App.tsx` now calls `src/test_raw_redraw.ts`. It requires the same prepared
throwaway note/page and raw-copy snapshot as the previous diagnostics. It refuses
to run if the current TOTALPATH no longer matches that snapshot. Backups and
payload comparisons stay on-device; only results, hashes, and screenshots are
collected on the host.

The installed PluginHost APK's Notes allowlist contains 79 methods. Notes has 87
dispatch entries. The complete inventory, native calls, permission checks,
broadcasts, and exported services are recorded in
`research/notes_refresh_api.json` in the plugin root. The inventory is complete
for these maps; JADX could not fully decompile some unrelated methods.

The on-disk `DIRTY` index value is an append-only cleanup counter according to
the supplied format research. The JNI implementation of `changeDirtyFlag(bool)`
writes a byte at offset `0x1378` in its native object and does no file I/O. Notes
enables it during initialization. Layer save flags also belong to native runtime
state. No dedicated PluginHost method to set a layer dirty or rebuild only its
bitmap was found. Native preload selects separate direct-bitmap and trail-redraw
paths; the inspected caller chooses redraw for changed canvas geometry/trimming.
Changing the stored DIRTY counter was not tested or claimed to force a redraw.

The tested SDK probes were:

| Probe | Stored bitmap | TOTALPATH |
| --- | --- | --- |
| `deletePageElements([], page)` | Unchanged; native result false | Unchanged |
| `batchUpdatePageElements([], [], page)` | Host rejected empty input, code 106 | Unchanged |
| `generateLayerPreviewImage` | Unchanged despite successful export | Unchanged |
| `generateNotePng` at scale 1 | Unchanged despite successful export | Unchanged |
| `modifyLayers` with identical main-layer metadata | Unchanged | Unchanged |
| `deletePageElements([2147483647], page)` after verifying that number is absent | Regenerated; dirty save | Native save reserialized payload |

The working workaround needs no lasso selection and sends no replacement stroke
objects. After the absent-number delete and SDK save, C++ copies the original
19,447-byte TOTALPATH back from the raw snapshot. It verifies the regenerated
MAINLAYER bitmap is byte-identical to the saved bitmap, publishes the verified
result at the same path with a fresh inode, and calls SDK `openFile` last. The
next invocation saves/reloads and compares both the bitmap and TOTALPATH again.
The diagnostic retains all probe cases for reproducible research; subsequent
invocations only check persistence.

### Verified raw restoration result, October 1, 2026

Native reload returned 0, the rendered RGB canvas matched the earlier lasso
result exactly, and a further SDK save/reload preserved all original TOTALPATH
bytes and the regenerated bitmap. SDK enumeration still returned three elements.
The Notes process remained PID 26256. Complete snapshots are retained under
`/storage/emulated/0/Note/snfiletools-test-backups/`; the successful restoration
backup is `totalpath-refresh-1790861875509.note.raw-redraw-1790865899572.note` and
its `.edited.note` companion contains the restored raw payload and new bitmap.

This is a firmware-dependent research workaround for the tested single-page
fixture. The SDK deletion handler clears undo/redo history. Native saving may
change other metadata, and later handwriting/edits may reserialize the raw trails
again. It is not a dedicated rebuild-only API.

Full package deployment, `npm run run` for both publication and persistence,
`npm run verify`, generator validation, and targeted TypeScript/ESLint checks
passed. The native suite passed with new assertions proving the read-only
TOTALPATH comparison detects a changed payload and recognizes byte restoration.
Evidence and results are in `build/snfiletools-raw-redraw-*` and
`build/snfiletools-empty-redraw-*`.

On this installed firmware, PluginHost's DEBUG broadcast receiver reads its
extras but performs no bundle installation. `npm run send` reports broadcast
delivery rather than an applied update. Use full `npm run deploy` for these tests;
restart PluginHost without clearing its data to load changed native/JS code,
then reopen the prepared note if the SDK current-file state is stale.

## Direct Binder cache-clear experiment

`src/test_clear_cache.ts` is the retained one-time diagnostic. Kotlin
`NoteRefresh.clearNoteFileCache` binds the exported Notes PluginClientService and
calls only `clearFileCache(path, zeroBasedPage)` using the observed request and
callback Parcel layouts. It awaits the callback, times out after 10 seconds, and
always unbinds. This tests the Notes dispatch entry absent from PluginHost's
normal Notes allowlist; it is firmware-specific.

On October 1, 2026, both direct calls succeeded (74 ms and 65 ms). Neither
`clearFileCache -> flashscreen -> save` nor
`clearFileCache -> reloadFile -> flashscreen -> save` rebuilt the blank bitmap
after the exact raw TOTALPATH copy. Bitmap bytes stayed unchanged, TOTALPATH
stayed byte-exact, native saving reported `isSave false`, and enumeration still
returned three elements. Thus direct Binder access works, but cache clearing
does not substitute for the absent-element deletion or lasso regeneration.

The previously visible note was backed up and restored on-device byte-for-byte
(SHA-256 `21752911547c92ac0477ea63573a2b943ec40f4496baf37b82f7850ebc65f052`),
then reopened successfully. Source note bytes stayed unchanged. The diagnostic
records completion in plugin-private state and then refuses to reset the fixture
on later runs. Results/logs/screenshots are in `build/snfiletools-clear-cache-*`.

## Active TOTALPATH insertion and redraw

The plugin button now calls `src/insert_totalpath.ts`. It saves and backs up the
currently open destination note, replaces PAGE1's TOTALPATH with the first
nonempty TOTALPATH from `/storage/emulated/0/Note/test.note`, verifies that the
bitmap bytes were untouched, clears the native cache and awaits SDK reload to
load the imported strokes. This copies the whole raw block; it
replaces the page's existing trail payload. The original complete note stays in
an on-device backup. The diagnostic requires page zero and a separate source.

One button press now completes the sequence:

1. Copy the exact source payload, clear cache and await `reloadFile` completion.
2. Verify that Notes loaded the imported element count and that element number
   2147483647 is absent. Await the direct Binder
   `clearFileCache` response, then delete that absent number through the SDK to
   trigger regeneration and save.
3. Restore the exact copied TOTALPATH while retaining the new bitmap, publish
   with a fresh inode, and reopen.
4. On the next launch, check the previous result after SDK save and reload.

Each launch starts a new backed-up insertion; previous diagnostic phase state
is not used to resume. There is no lasso selection and no submission of
replacement JS stroke objects. The SDK deletion handler clears undo/redo
history. Results are logged through `console.log`.

The earlier three-launch version stopped before redraw after its first launch.
The first single-launch attempt also exposed a Notes native crash during bitmap
recycling. APK inspection showed that deletion schedules UI redraw while
clearing bitmap caches on its Binder thread. Clearing the cache separately and
awaiting completion before deletion avoided that observed crash in both live
tests. This Binder protocol is specific to the tested firmware.

Two consecutive single-launch tests passed on October 1, 2026. The first began
with a blank bitmap and displayed two strokes plus the legacy "testing" text
box without touching the lasso. It copied 19,447 raw payload bytes, changed the
stored bitmap, and preserved exact TOTALPATH bytes and the regenerated bitmap
after SDK save/reload. The second run exercised cleanup with an existing bitmap
and passed again. Notes retained PID 24355 throughout both runs, and the source
hash stayed unchanged. Full operations took 8,065 ms and 7,924 ms, including
6,500 ms of deliberate waits each. `npm run deploy`, both `npm run run` tests,
`npm run verify`, generator validation, targeted TypeScript, and targeted ESLint
passed. The first original-note backup is
`/storage/emulated/0/Note/snfiletools-test-backups/insert-totalpath-1790877187034.note`.
Scalar results and screenshot are `build/snfiletools-single-insert-result.json`
and `build/snfiletools-single-insert-safe-after.png`.

### Intermediate version with shorter waits

An intermediate implementation removed 4,500 ms of redundant waits.
It retained a 1,000 ms loading buffer after each `openFile`, because that API
acknowledges the intent before loading finishes. Deletion posts redraw to the
same Notes UI handler used by save; the redraw waits for layer rendering before
the queued save runs. The save response arrives after saving, and `reloadFile`
responds through the page-load completion callback. Those completion responses
replace the former redraw, post-save, and post-reload delays.

Three live runs passed in 3,899 ms, 3,569 ms and 3,341 ms, including the full
save/reload verification. The first started from the original blank bitmap;
Notes set the regenerated page bitmap at 1,841 ms and finished redraw at
1,867 ms after copy started. This measures Notes rendering, not physical e-ink
panel latency. All runs preserved exact raw bytes and bitmap after save/reload,
retained Notes PID 26709, and left the source unchanged. The first two npm launch
checks timed out despite successful plugin completion: counts from repeated
logcat ring-buffer dumps missed the acknowledgment. The Bash run script now
captures PluginManager events live before tapping. The third `npm run run`
exited successfully. `npm run deploy`, `npm run verify`, targeted TypeScript,
targeted ESLint and `bash -n scripts/runPlugin.sh` also passed.

Evidence is in `build/snfiletools-fast-insert-*`; the scalar result is
`build/snfiletools-fast-insert-result.json`.

### Current version without fixed waits

The current version replaces the initial inode publication/open with direct
cache clear plus SDK reload. The reload callback waits for page loading; SDK
element enumeration verifies that Notes loaded all three imported records.
Pre-clearing before absent-number deletion remains necessary to avoid the
bitmap recycling race. After redraw/save and exact raw-byte restoration, one
final fresh-inode publication/open is still required to display the new bitmap.
It is the last action, so no loading timer is needed in the insertion itself.
Previous-result checks run during the next launch's save/reload sequence and
log scalars; user edits to the previous result do not prevent a new insertion.

Two runs passed on October 1, 2026: edit processing took 1,393 ms and 1,310 ms.
Notes set the final displayed bitmap at 2,235 ms and 2,171 ms from the start of
copying, and finished loading at 2,314 ms and 2,246 ms. These are app log timings,
not measured physical e-ink latency. The first began with a blank note and the
screenshots confirm two strokes plus "testing", without a lasso. The second
confirmed the prior exact TOTALPATH bytes survived SDK save and the prior
regenerated bitmap survived SDK reload. Notes retained PID 31121 throughout
both runs and the source hash stayed unchanged. Both `npm run run` invocations,
`npm run deploy`, `npm run verify`, targeted TypeScript and ESLint passed.

Rejected alternatives: `canHandwrite` changed from blocked to ready before
loading completed, and failed the persistence check. A cache/reload-only flow
saved the correct bitmap but left the screen blank when merely closing the
plugin view. Thus final reopen is retained. No fixed sleep, readiness polling,
or `closePluginView` call remains in the active flow.

Logs are in `build/snfiletools-ready-insert-device.log`; successful-run records
and screenshots are in `build/snfiletools-final-insert-*`. The structured result
is `build/snfiletools-final-insert-result.json`.

`copyPageTotalPath(destination, source, backup, destinationPageKey, sourcePageKey)`
resolves both pages explicitly. A zero-offset TOTALPATH on a blank source page
is copied as an empty four-byte count. It never falls back to another page.
