const {getDefaultConfig, mergeConfig} = require('@react-native/metro-config');
const path = require('path');

const escapeRegExp = value => value.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
// Generated packages, native build trees and publication snapshots are not JS
// inputs. Excluding this project's build directory avoids redundant watchers.
const config = {
  resolver: {
    blockList: new RegExp(
      '^' + escapeRegExp(path.join(__dirname, 'build')) + String.raw`[\\/]`,
    ),
  },
};

module.exports = mergeConfig(getDefaultConfig(__dirname), config);
