class Module {
  constructor(path) {
    this.path = path;
    this.exports = {};
    this.loaded = false;
  }

  static _cache = new Map();

  static require(path) {
    if (Module._cache.has(path)) {
      const module = Module._cache.get(path);
      return module.exports;
    }

    const module = new Module(path);
    const exports = module.exports;

    Module._cache.set(path, module);
    
    const { func } = load(path);
    module.loaded = true;

    // __filename and __dirname will be implemented later
    func(exports, Module.require, module, path, path);
    
    Module._cache.set(path, module);

    return module.exports;
  }
}

Module.require(__entryfile);
