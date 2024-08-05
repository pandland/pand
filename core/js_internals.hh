/* AUTO GENERATED FILE - DO NOT EDIT IT MANUALLY */
#ifndef JS_MODULES_H
#define JS_MODULES_H

#include <unordered_map>
#include <string>

std::unordered_map<std::string, std::string> js_internals = {
    {"std:assert", "export class AssertionError extends Error {\n  constructor(message) {\n    super(message);\n    this.name = 'AssertionError'\n  }\n}\n\nexport function assert(condition, message) {\n  if (!condition) {\n      throw new AssertionError(message || \"Assertion failed\");\n  }\n}\n\nexport function assertStrictEqual(actual, expected, message) {\n  if (!Object.is(actual, expected)) {\n    throw new AssertionError(message || \"Assert strict equal failed\");\n  }\n}\n\nexport function assertThrows(func, errClass = Error, message) {\n  try {\n    func();\n  } catch (err) {\n    if (err instanceof errClass) {\n      return;\n    }\n  }\n\n  throw new AssertionError(message || \"Function does not throw Error\");\n}\n"},
    {"std:bootstrap", "console.log = println;\nconsole.error = println;\n"}
};

#endif // JS_MODULES_H
