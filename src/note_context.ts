import {PluginCommAPI} from 'sn-plugin-lib';

export type NoteContext = {path: string; page: number};

// sn-plugin-lib returns response objects; native module exceptions instead
// reject their promises and are handled once at the UI boundary.
export function result<T>(
  response: Object | null | undefined,
  operation: string,
): T {
  const value = response as {
    success: boolean;
    result: T;
    error?: {message: string};
  };
  if (!value?.success || value.result == null || value.result === false) {
    throw new Error(value?.error?.message ?? `${operation} failed`);
  }
  return value.result;
}

export async function getCurrentNotePath(): Promise<string> {
  const path = result<string>(
    await PluginCommAPI.getCurrentFilePath(),
    'current note',
  );
  if (!path.endsWith('.note')) {
    throw new Error('Open a note before replacing its page.');
  }
  return path;
}

export async function getNoteContext(): Promise<NoteContext> {
  const path = await getCurrentNotePath();
  const page = result<number>(
    await PluginCommAPI.getCurrentPageNum(),
    'current page',
  );
  if (!Number.isInteger(page) || page < 0) {
    throw new Error('The current note page is unavailable.');
  }
  return {path, page};
}
