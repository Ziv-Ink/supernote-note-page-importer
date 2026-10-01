import {
  PluginCommAPI,
  PluginFileAPI,
  PluginManager,
  PluginNoteAPI,
} from 'sn-plugin-lib';
import Snfiletools from 'snfiletools';
import {ensureFileWritePermission} from './permissions';
import {getCurrentNotePath, NoteContext, result} from './note_context';

let running = false;

// Reload responses wait for Notes' page-load completion. Keep the inode stable
// while copying/redrawing, then publish/reopen once to display the bitmap.
export async function insertTotalPath(
  source: string,
  destination: NoteContext,
): Promise<void> {
  if (running) {
    throw new Error('A page replacement is already running.');
  }
  running = true;
  try {
    const {path, page} = destination;
    if (!source.endsWith('.note')) {
      throw new Error('Select a .note file.');
    }
    if (source === path) {
      throw new Error('Select a different source note.');
    }
    const ensureSameNote = async () => {
      if (
        (await getCurrentNotePath()) !== path ||
        result<number>(
          await PluginCommAPI.getCurrentPageNum(),
          'current page',
        ) !== page
      ) {
        throw new Error('The open note/page changed. Reopen the picker.');
      }
    };
    await ensureSameNote();
    const directory = await PluginManager.getPluginDirPath();
    if (!directory) {
      throw new Error('Missing plugin directory');
    }
    await ensureFileWritePermission();
    await Snfiletools.setJournalDirectory(`${directory}/snfiletools-journal`);
    result(await PluginNoteAPI.saveCurrentNote(), 'save before insertion');
    await ensureSameNote();
    // Refresh readers after the SDK save; retain a complete on-device backup.
    await Snfiletools.setJournalDirectory(
      `${directory}/snfiletools-refresh-journal`,
    );
    await Snfiletools.setJournalDirectory(`${directory}/snfiletools-journal`);
    const pageKey = `PAGE${page + 1}`;
    const backup = `/storage/emulated/0/Note/snfiletools-page-backups/replace-page-${Date.now()}.note`;
    const rawSnapshot = `${backup}.edited.note`;
    const unchanged = () =>
      Snfiletools.testTotalPathUnchanged(path, rawSnapshot, pageKey);
    const started = Date.now();
    console.log(
      `[SNfiletools] INSERT destination=${path}, source=${source}, backup=${backup}`,
    );
    const copied = await Snfiletools.copyPageTotalPath(
      path,
      source,
      backup,
      pageKey,
      'PAGE1',
    );
    console.log(`[SNfiletools] INSERT ${copied}`);
    const importedCount = Number(
      copied.match(/ imported = (\d+) trails;/)?.[1],
    );
    if (!Number.isInteger(importedCount) || importedCount < 0) {
      throw new Error('The copied payload count is invalid');
    }
    if (!(await Snfiletools.testBitmapUnchanged(path, backup, pageKey))) {
      throw new Error('Raw insertion unexpectedly changed the bitmap');
    }
    if (!(await Snfiletools.clearNoteFileCache(path, page))) {
      throw new Error(
        'Could not clear page cache before loading inserted bytes',
      );
    }
    result(await PluginCommAPI.reloadFile(), 'load inserted bytes');
    await ensureSameNote();
    if (!(await unchanged())) {
      throw new Error(
        'Inserted TOTALPATH changed before redraw; backup retained',
      );
    }
    console.log('[SNfiletools] INSERT CONTINUING after reload completion');
    const elements = result<{numInPage: number}[]>(
      await PluginFileAPI.getElements(page, path),
      'inserted elements',
    );
    const absentNumber = 2147483647;
    // TOTALPATH can also contain erased/hidden records that getElements
    // omits. Verify raw bytes above; do not equate record and element counts.
    if (!Array.isArray(elements)) {
      throw new Error('Notes returned invalid page elements');
    }
    if (elements.some(element => element.numInPage === absentNumber)) {
      throw new Error('The redraw probe number exists; refusing deletion');
    }
    // Notes' delete handler clears page bitmaps on its Binder thread while
    // scheduling redraw on the UI thread. Clear them first and await completion
    // so the two threads cannot concurrently recycle the same bitmap.
    if (!(await Snfiletools.clearNoteFileCache(path, page))) {
      throw new Error('Could not clear page bitmaps before redraw');
    }
    result(
      await PluginCommAPI.deletePageElements([absentNumber], page),
      'trigger redraw',
    );
    await ensureSameNote();
    // Notes queues redraw and save on the same UI handler. Awaiting save
    // therefore waits for regeneration too; no extra timer is needed.
    result(await PluginNoteAPI.saveCurrentNote(), 'save regenerated bitmap');
    console.log(
      `[SNfiletools] INSERT REDRAW: bitmapChanged=${!(await Snfiletools.testBitmapUnchanged(
        path,
        backup,
        pageKey,
      ))}, totalpathPreservedBySdk=${await unchanged()}`,
    );
    const renderedBackup = `${backup}.redraw-${Date.now()}.note`;
    console.log(
      `[SNfiletools] INSERT ${await Snfiletools.copyPageTotalPath(
        path,
        rawSnapshot,
        renderedBackup,
        pageKey,
        pageKey,
      )}`,
    );
    if (
      !(await unchanged()) ||
      !(await Snfiletools.testBitmapUnchanged(path, renderedBackup, pageKey))
    ) {
      throw new Error(
        'Exact TOTALPATH restore failed to retain regenerated bitmap',
      );
    }
    const renderedSnapshot = `${renderedBackup}.edited.note`;
    await Snfiletools.testReopenSnapshot(path, renderedSnapshot);
    // Final open invalidates Notes' displayed bitmap by inode. It is the last
    // action so Notes displays the regenerated page.
    result(
      await PluginFileAPI.openFile(path, page),
      'display regenerated note',
    );
    console.log(
      `[SNfiletools] INSERT APPLIED: raw TOTALPATH byte-exact; regenerated bitmap retained; elements=${
        elements.length
      }; records=${importedCount}; elapsedMs=${Date.now() - started}; backup=${backup}`,
    );
  } finally {
    running = false;
  }
}
