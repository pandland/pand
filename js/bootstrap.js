class Module {
  constructor(path) {
    this.path = path;
    this.exports = {};
  }

  static _cache = new Map();

  static require(path) {
    if (Module._cache.has(path)) {
      println(`Cache hit: ${path}`);
      const module = Module._cache.get(path);
      return module.exports;
    }

    const module = new Module(path);
    module.exports = {};

    const exports = module.exports;

    Module._cache.set(path, module);
    
    const func = load(path);
    // __filename and __dirname will be implemented later
    func(exports, Module.require, module, path, path);
    
    Module._cache.set(path, module);

    return module.exports;
  }
}

Module.require(_ENTRYFILE);
