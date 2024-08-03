// TODO: it will be our entrypoint - wrapper for the script
println(`Running: ${_ENTRYFILE}`);

class Module {
  exports = {};
  path = '';

  static require(path) {
    const module = new Module();
    module.path = path;
    module.exports = {};

    const exports = module.exports;

    const func = load(path);
    // __filename and __dirname will be implemented later
    func(exports, Module.require, path, path);

    return module.exports;
  }
}

Module.require(_ENTRYFILE);
