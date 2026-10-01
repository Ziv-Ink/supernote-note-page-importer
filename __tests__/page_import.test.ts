import {PluginCommAPI, PluginFileAPI, PluginNoteAPI} from 'sn-plugin-lib';
import Snfiletools from 'snfiletools';
import {insertTotalPath} from '../src/insert_totalpath';

jest.mock('sn-plugin-lib', () => ({
  PluginCommAPI: {
    getCurrentFilePath: jest.fn(),
    getCurrentPageNum: jest.fn(),
    reloadFile: jest.fn(),
    deletePageElements: jest.fn(),
  },
  PluginFileAPI: {getElements: jest.fn(), openFile: jest.fn()},
  PluginNoteAPI: {saveCurrentNote: jest.fn()},
  PluginManager: {
    getPluginDirPath: jest.fn().mockResolvedValue('/plugin'),
    hasPermission: jest.fn().mockResolvedValue(2),
  },
}));
jest.mock('snfiletools', () => ({
  setJournalDirectory: jest.fn(),
  copyPageTotalPath: jest.fn(),
  testTotalPathUnchanged: jest.fn().mockResolvedValue(true),
  testBitmapUnchanged: jest.fn().mockResolvedValue(true),
  clearNoteFileCache: jest.fn().mockResolvedValue(true),
  testReopenSnapshot: jest.fn(),
}));
const mock = (fn: unknown) => fn as jest.Mock;
const success = (value: unknown) => ({success: true, result: value});
const destination = {path: '/Note/destination.note', page: 1};

beforeEach(() => {
  jest.clearAllMocks();
  mock(PluginCommAPI.getCurrentFilePath).mockResolvedValue(success(destination.path));
  mock(PluginCommAPI.getCurrentPageNum).mockResolvedValue(success(destination.page));
  mock(PluginCommAPI.reloadFile).mockResolvedValue(success(true));
  mock(PluginCommAPI.deletePageElements).mockResolvedValue(success(true));
  mock(PluginNoteAPI.saveCurrentNote).mockResolvedValue(success(true));
  mock(PluginFileAPI.openFile).mockResolvedValue(success(true));
  mock(PluginFileAPI.getElements).mockResolvedValue(success([{numInPage: 1}]));
  mock(Snfiletools.copyPageTotalPath).mockResolvedValue('0 existing + 1 imported = 1 trails;');
  jest.spyOn(console, 'log').mockImplementation(() => {});
});
afterEach(() => jest.restoreAllMocks());

test('copies selected source PAGE1 into captured PAGE2 and restores that same page', async () => {
  await insertTotalPath('/Note/chosen.note', destination);
  const [first, restore] = mock(Snfiletools.copyPageTotalPath).mock.calls;
  expect(first).toEqual([destination.path, '/Note/chosen.note', expect.any(String), 'PAGE2', 'PAGE1']);
  expect(restore).toEqual([destination.path, `${first[2]}.edited.note`, expect.any(String), 'PAGE2', 'PAGE2']);
  expect(PluginCommAPI.deletePageElements).toHaveBeenCalledWith([2147483647], 1);
  const clearOrders = mock(Snfiletools.clearNoteFileCache).mock.invocationCallOrder;
  expect(clearOrders[1]).toBeLessThan(mock(PluginCommAPI.deletePageElements).mock.invocationCallOrder[0]);
  expect(mock(Snfiletools.testReopenSnapshot).mock.invocationCallOrder[0]).toBeLessThan(mock(PluginFileAPI.openFile).mock.invocationCallOrder[0]);
  expect(PluginFileAPI.openFile).toHaveBeenCalledWith(destination.path, 1);
});

test('blank source page clears the destination and redraws with zero elements', async () => {
  mock(Snfiletools.copyPageTotalPath).mockResolvedValue('0 existing + 0 imported = 0 trails;');
  mock(PluginFileAPI.getElements).mockResolvedValue(success([]));
  await insertTotalPath('/Note/blank-first-page.note', destination);
  expect(PluginCommAPI.deletePageElements).toHaveBeenCalledTimes(1);
  expect(PluginFileAPI.openFile).toHaveBeenCalledWith(destination.path, 1);
});

test('refuses a changed destination before writing', async () => {
  mock(PluginCommAPI.getCurrentPageNum).mockResolvedValue(success(0));
  await expect(insertTotalPath('/Note/chosen.note', destination)).rejects.toThrow('open note/page changed');
  expect(Snfiletools.copyPageTotalPath).not.toHaveBeenCalled();
  expect(PluginNoteAPI.saveCurrentNote).not.toHaveBeenCalled();
});

test('refuses the same source and destination', async () => {
  await expect(insertTotalPath(destination.path, destination)).rejects.toThrow('different source note');
  expect(Snfiletools.copyPageTotalPath).not.toHaveBeenCalled();
});

test('does not delete when the reserved redraw number actually exists', async () => {
  mock(PluginFileAPI.getElements).mockResolvedValue(success([{numInPage: 2147483647}]));
  await expect(insertTotalPath('/Note/chosen.note', destination)).rejects.toThrow('refusing deletion');
  expect(PluginCommAPI.deletePageElements).not.toHaveBeenCalled();
});

test('does not treat hidden or erased TOTALPATH records as visible elements', async () => {
  mock(Snfiletools.copyPageTotalPath).mockResolvedValue('0 existing + 5 imported = 5 trails;');
  mock(PluginFileAPI.getElements).mockResolvedValue(success([]));
  await insertTotalPath('/Note/erased.note', destination);
  expect(PluginFileAPI.openFile).toHaveBeenCalledWith(destination.path, 1);
});
