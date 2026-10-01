/* global globalThis */
export class SupernoteError extends Error {
  constructor(code, message) {
    super(message);
    this.name = 'SupernoteError';
    Object.defineProperty(this, 'code', {value: code, enumerable: true});
  }
}

const ERROR_CONSTRUCTOR_PROPERTY = '__supernoteErrorConstructor';
const CPP_OBJECT_INFO_PROPERTY = '__supernoteCppObjectInfo';
const JVM_OBJECT_INFO_PROPERTY = '__supernoteJvmObjectInfo';
const RUNTIME_UNAVAILABLE_ERROR = 'runtime-unavailable: @supernote/runtime is not loaded for snfiletools. Add @supernote/runtime as a direct dependency and rebuild the plugin.';
const FEATURE_UNAVAILABLE_ERROR = 'feature-unavailable: snfiletools is not loaded in the Supernote runtime. Add snfiletools as a direct dependency and rebuild the plugin.';

const VALIDATION_REASONS = new Set([
  'ARITY_MISMATCH',
  'TYPE_MISMATCH',
  'NOMINAL_MISMATCH',
  'MISSING_FIELD',
  'INVALID_ENUM',
  'OUT_OF_RANGE',
  'LIMIT_EXCEEDED',
]);

function currentFeature() {
  const runtime = globalThis.__supernoteModule;
  if (!runtime || typeof runtime.feature !== 'function') {
    return {status: 'runtime-unavailable'};
  }
  try {
    const value = runtime.feature('supernote:feature:7e680f69f87879eb');
    if (!value || (typeof value !== 'object' && typeof value !== 'function')) {
      return {status: 'feature-unavailable'};
    }
    return {status: 'available', value};
  } catch (_error) {
    return {status: 'feature-unavailable'};
  }
}

export function getFeatureStatus() {
  return currentFeature().status;
}

export function isFeatureAvailable() {
  return getFeatureStatus() === 'available';
}

export function nativeObjectInfo(value) {
  const current = currentFeature();
  if (current.status !== 'available') {
    return undefined;
  }
  for (const property of [CPP_OBJECT_INFO_PROPERTY, JVM_OBJECT_INFO_PROPERTY]) {
    const inspect = current.value[property];
    if (typeof inspect !== 'function') {
      continue;
    }
    const info = inspect(value);
    if (info !== undefined) {
      return info;
    }
  }
  return undefined;
}

function hasValidationDetails(value) {
  return Boolean(
    value &&
      VALIDATION_REASONS.has(value.reason) &&
      typeof value.path === 'string' &&
      typeof value.expected === 'string' &&
      typeof value.actual === 'string',
  );
}

export function isSupernoteTypeError(value) {
  return value instanceof TypeError && hasValidationDetails(value);
}

export function isSupernoteRangeError(value) {
  return value instanceof RangeError && hasValidationDetails(value);
}

function requireFeature() {
  const current = currentFeature();
  if (current.status === 'runtime-unavailable') {
    throw new Error(RUNTIME_UNAVAILABLE_ERROR);
  }
  if (current.status === 'feature-unavailable') {
    throw new Error(FEATURE_UNAVAILABLE_ERROR);
  }
  const value = current.value;
  if (value[ERROR_CONSTRUCTOR_PROPERTY] !== SupernoteError) {
    Object.defineProperty(value, ERROR_CONSTRUCTOR_PROPERTY, {
      configurable: true,
      enumerable: false,
      value: SupernoteError,
      writable: false,
    });
  }
  return value;
}

const feature = new Proxy(
  {},
  {
    get(_target, property) {
      if (property === ERROR_CONSTRUCTOR_PROPERTY ||
          property === CPP_OBJECT_INFO_PROPERTY ||
          property === JVM_OBJECT_INFO_PROPERTY) {
        return undefined;
      }
      return requireFeature()[property];
    },
    has(_target, property) {
      if (property === ERROR_CONSTRUCTOR_PROPERTY ||
          property === CPP_OBJECT_INFO_PROPERTY ||
          property === JVM_OBJECT_INFO_PROPERTY) {
        return false;
      }
      return property in requireFeature();
    },
    ownKeys() {
      return Reflect.ownKeys(requireFeature()).filter(
        property => property !== ERROR_CONSTRUCTOR_PROPERTY &&
          property !== CPP_OBJECT_INFO_PROPERTY &&
          property !== JVM_OBJECT_INFO_PROPERTY,
      );
    },
    getOwnPropertyDescriptor(_target, property) {
      if (property === ERROR_CONSTRUCTOR_PROPERTY ||
          property === CPP_OBJECT_INFO_PROPERTY ||
          property === JVM_OBJECT_INFO_PROPERTY) {
        return undefined;
      }
      const descriptor = Object.getOwnPropertyDescriptor(
        requireFeature(),
        property,
      );
      return descriptor ? {...descriptor, configurable: true} : undefined;
    },
  },
);

export default feature;
