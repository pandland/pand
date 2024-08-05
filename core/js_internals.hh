/* AUTO GENERATED FILE - DO NOT EDIT IT MANUALLY */
#ifndef JS_MODULES_H
#define JS_MODULES_H

#include <unordered_map>
#include <string>

std::unordered_map<std::string, std::string> js_internals = {
    {"std:assert", "export function assert(condition, message) {\n  if (!condition) {\n      throw new Error(message || \"Assertion failed\");\n  }\n}\n"},
    {"std:bootstrap", "console.log = println;\nconsole.error = println;\n"}
};

#endif // JS_MODULES_H
