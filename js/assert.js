export class AssertionError extends Error {
  constructor(message) {
    super(message);
    this.name = 'AssertionError'
  }
}

export function assert(condition, message) {
  if (!condition) {
      throw new AssertionError(message || "Assertion failed");
  }
}

export function assertStrictEqual(actual, expected, message) {
  if (!Object.is(actual, expected)) {
    throw new AssertionError(message || "Assert strict equal failed");
  }
}

export function assertThrows(func, errClass = Error, message) {
  try {
    func();
  } catch (err) {
    if (err instanceof errClass) {
      return;
    }
  }

  throw new AssertionError(message || "Function does not throw Error");
}
