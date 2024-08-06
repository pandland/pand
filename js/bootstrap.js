console.log = (arg) => { Runtime.print(`${arg}\n`); }
console.error = (arg) => { Runtime.printerr(`${arg}\n`); }

globalThis.require = (module) => {
  if (typeof module === 'string') {
    throw new Error(`CommonJS modules are not supported. Use ES6 'import "${module}"' syntax.`);
  }
  throw new Error('CommonJS modules are not supported.');
}
