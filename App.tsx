import React, {useEffect, useRef, useState} from 'react';
import {Pressable, StyleSheet, Text, View} from 'react-native';
import {PluginManager, RattaFileSelector} from 'sn-plugin-lib';
import {insertTotalPath} from './src/insert_totalpath';
import {getNoteContext, NoteContext} from './src/note_context';
import {ensureFileReadPermission} from './src/permissions';

const filename = (path: string) => path.slice(path.lastIndexOf('/') + 1);

function App(): React.JSX.Element {
  const [visible, setVisible] = useState(false);
  const [destination, setDestination] = useState<NoteContext>();
  const [source, setSource] = useState('');
  const [busy, setBusy] = useState(false);
  const [message, setMessage] = useState('');
  const active = useRef(false);

  async function perform(operation: () => Promise<void>) {
    if (active.current) {
      return;
    }
    active.current = true;
    setBusy(true);
    setMessage('');
    try {
      await operation();
    } catch (error) {
      const text = error instanceof Error ? error.message : String(error);
      console.error('[SNfiletools] PAGE IMPORT FAILED:', error);
      setMessage(text);
      setVisible(true);
      await PluginManager.showPluginView();
    } finally {
      active.current = false;
      setBusy(false);
    }
  }

  useEffect(() => {
    const subscription = PluginManager.registerButtonListener({
      onButtonPress: () => {
        perform(async () => {
          setSource('');
          setDestination(undefined);
          setVisible(true);
          const current = await getNoteContext();
          setDestination(current);
          await PluginManager.showPluginView();
          console.log(
            `[SNfiletools] PICKER OPEN: ${current.path}, page=${
              current.page + 1
            }`,
          );
        });
      },
    });
    return () => subscription.remove();
    // The listener reads the in-flight ref, not a stale busy state.
  }, []);

  const choose = () =>
    perform(async () => {
      await ensureFileReadPermission();
      const selected = await RattaFileSelector.selectFile({
        selectType: 1,
        suffixList: ['note'],
        maxNum: 1,
        title: 'Choose source note',
        rightButtonText: 'Select',
        needSelectFolder: '/storage/emulated/0/Note',
      });
      if (selected?.[0]) {
        if (selected[0] === destination?.path) {
          throw new Error('Select a different source note.');
        }
        setSource(selected[0]);
        console.log(`[SNfiletools] SOURCE SELECTED: ${selected[0]}`);
      }
      await PluginManager.showPluginView();
    });

  const replace = () =>
    perform(async () => {
      if (!destination || !source) {
        throw new Error('Choose a source note first.');
      }
      setMessage('Replacing page…');
      await insertTotalPath(source, destination);
      setVisible(false);
      setSource('');
    });

  const close = () =>
    perform(async () => {
      setVisible(false);
      setSource('');
      await PluginManager.closePluginView();
      console.log('[SNfiletools] PICKER CANCELLED');
    });

  if (!visible) {
    return <View />;
  }
  return (
    <View style={styles.screen}>
      <View style={styles.card}>
        <Text style={styles.title}>Replace note page</Text>
        <Text style={styles.label}>Current note</Text>
        <Text style={styles.value}>
          {destination ? filename(destination.path) : 'No open note'}
        </Text>
        <Text style={styles.description}>
          Page {destination ? destination.page + 1 : '—'}
        </Text>
        <Text style={styles.description}>
          Replace this page’s strokes with page one from another note. A backup
          is saved on the device.
        </Text>
        <Pressable
          accessibilityRole="button"
          disabled={busy || !destination}
          onPress={choose}
          style={[styles.button, busy && styles.disabled]}>
          <Text style={styles.buttonText}>Choose note</Text>
        </Pressable>
        <Text style={styles.value}>
          {source ? filename(source) : 'No source selected'}
        </Text>
        {!!message && (
          <Text accessibilityRole="alert" style={styles.message}>
            {message}
          </Text>
        )}
        <View style={styles.actions}>
          <Pressable
            accessibilityRole="button"
            disabled={busy}
            onPress={close}
            style={styles.button}>
            <Text style={styles.buttonText}>Cancel</Text>
          </Pressable>
          <Pressable
            accessibilityRole="button"
            disabled={busy || !source || !destination}
            onPress={replace}
            style={[
              styles.button,
              styles.primary,
              (busy || !source) && styles.disabled,
            ]}>
            <Text style={styles.primaryText}>
              {busy ? 'Working…' : 'Replace page'}
            </Text>
          </Pressable>
        </View>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  screen: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#eee',
    padding: 32,
  },
  card: {
    width: '100%',
    maxWidth: 700,
    backgroundColor: '#fff',
    borderWidth: 2,
    borderColor: '#000',
    padding: 32,
    gap: 20,
  },
  title: {fontSize: 32, fontWeight: '700', color: '#000'},
  label: {fontSize: 20, fontWeight: '600', color: '#000'},
  value: {fontSize: 24, color: '#000'},
  description: {fontSize: 22, lineHeight: 30, color: '#000'},
  button: {
    borderWidth: 2,
    borderColor: '#000',
    paddingVertical: 18,
    paddingHorizontal: 24,
    alignItems: 'center',
  },
  buttonText: {fontSize: 24, color: '#000'},
  primary: {backgroundColor: '#000'},
  primaryText: {fontSize: 24, color: '#fff'},
  disabled: {opacity: 0.4},
  actions: {flexDirection: 'row', justifyContent: 'space-between', gap: 20},
  message: {fontSize: 22, lineHeight: 30, color: '#000'},
});

export default App;
