#ifndef JS_MODULES_H
#define JS_MODULES_H

#include <unordered_map>
#include <string>

std::unordered_map<std::string, std::string> js_internals = {
    {"bootstrap", "class Module {\n  exports = {};\n\n  constructor(path) {\n    this.path = path;\n  }\n\n  static _cache = new Map();\n\n  static require(path) {\n    if (this._cache.has(path)) {\n      println(`Cache hit: ${path}`);\n      const module = this._cache.get(path);\n      return module.exports;\n    }\n\n    const module = new Module(path);\n    module.exports = {};\n\n    const exports = module.exports;\n\n    const func = load(path);\n    // __filename and __dirname will be implemented later\n    func(exports, Module.require.bind(Module), module, path, path);\n    \n    this._cache.set(path, module);\n\n    return module.exports;\n  }\n}\n\nModule.require(_ENTRYFILE);\n"}
};

#endif // JS_MODULES_H
