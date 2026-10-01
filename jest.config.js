module.exports = {
  preset: 'react-native',
  // Generated bundles and clean publication copies are not test inputs.
  modulePathIgnorePatterns: ['<rootDir>/build/'],
  testPathIgnorePatterns: ['/node_modules/', '<rootDir>/build/'],
};
