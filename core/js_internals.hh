/* AUTO GENERATED FILE - DO NOT EDIT IT MANUALLY */
#ifndef JS_MODULES_H
#define JS_MODULES_H

#include <unordered_map>
#include <string>

std::unordered_map<std::string, std::string> js_internals = {
    {"std:assert", "export class AssertionError extends Error {\n  constructor(message) {\n    super(message);\n    this.name = 'AssertionError'\n  }\n}\n\nexport function assert(condition, message) {\n  if (!condition) {\n      throw new AssertionError(message || \"Assertion failed\");\n  }\n}\n\nexport function assertStrictEqual(actual, expected, message) {\n  if (!Object.is(actual, expected)) {\n    throw new AssertionError(message || \"Assert strict equal failed\");\n  }\n}\n\nexport function assertThrows(func, errClass = Error, message) {\n  try {\n    func();\n  } catch (err) {\n    if (err instanceof errClass) {\n      return;\n    }\n  }\n\n  throw new AssertionError(message || \"Function does not throw Error\");\n}\n"},
    {"std:net", "const net = Runtime.bind(\"tcp\");\n\nclass Socket {\n  constructor(handle) {\n    this._handle = handle;\n    this.closed = false;\n  }\n\n  read(cb) {\n    this._handle.onread = cb;\n  }\n\n  close() {\n    if (this._handle) {\n      this._handle.close();\n      this.closed = true;\n    }\n    // otherwise - ignore and make it safe to call multiple times\n  }\n\n  write(data) {\n    if (this._handle) {\n      this._handle.write(data);\n    }\n  }\n}\n\nexport const tcpListen = (callback, port) => {\n  net.tcpListen((handle) => {\n    const socket = new Socket(handle);\n    handle.onclose = () => {\n      socket._handle = null;\n    }\n    handle.onread = (chunk) => {} // user should call socket.read() to overwrite this callback\n    \n    callback(socket);\n  }, port);\n};\n"},
    {"std:bootstrap", "console.log = (arg) => { Runtime.print(`${arg}\n`); }\nconsole.error = (arg) => { Runtime.printerr(`${arg}\n`); }\n\nglobalThis.require = (module) => {\n  if (typeof module === 'string') {\n    throw new Error(`CommonJS modules are not supported. Use ES6 'import \"${module}\"' syntax.`);\n  }\n  throw new Error('CommonJS modules are not supported.');\n}\n"}
};

#endif // JS_MODULES_H
