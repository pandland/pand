class Module {
  exports = {};

  constructor(path) {
    this.path = path;
  }

  static _cache = new Map();

  static require(path) {
    if (this._cache.has(path)) {
      println(`Cache hit: ${path}`);
      const module = this._cache.get(path);
      return module.exports;
    }

    const module = new Module(path);
    module.exports = {};

    const exports = module.exports;

    const func = load(path);
    // __filename and __dirname will be implemented later
    func(exports, Module.require.bind(Module), module, path, path);
    
    this._cache.set(path, module);

    return module.exports;
  }
}

Module.require(_ENTRYFILE);
