import {PluginManager} from 'sn-plugin-lib';

async function ensureFilePermission(access: 'READ' | 'WRITE'): Promise<number> {
  const permission = `plugin.permission.FILE:${access}`;
  let status = await PluginManager.hasPermission(permission);
  if (![1, 2].includes(status)) {
    status = await PluginManager.requestPermission(
      permission,
      `${access} access is required to read the selected note and replace the current page.`,
    );
  }
  if (![1, 2].includes(status)) {
    throw new Error(`File ${access} permission was not granted`);
  }
  return status;
}

export function ensureFileReadPermission(): Promise<number> {
  return ensureFilePermission('READ');
}

export function ensureFileWritePermission(): Promise<number> {
  return ensureFilePermission('WRITE');
}
