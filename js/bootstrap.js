import { inspectMany } from 'std:inspect';

console.log = (...args) => { Runtime.print(inspectMany(args, {}) + "\n"); }
console.error = (...args) => { Runtime.printerr(inspectMany(args, {}) + "\n"); }

globalThis.require = (module) => {
  if (typeof module === 'string') {
    throw new Error(`CommonJS modules are not supported. Use ES6 'import "${module}"' syntax.`);
  }
  throw new Error('CommonJS modules are not supported.');
}

const { Timer } = Runtime.bind("timer");

globalThis.setTimeout = (cb, delay, ...args) => {
  const timer = new Timer();
  timer.onTimeout = () => {
    cb(...args);
  }

  return timer.setTimeout(delay || 0);
}

globalThis.setInterval = (cb, delay, ...args) => {
  const timer = new Timer();
  timer.onTimeout = () => {
    cb(...args);
  }

  return timer.setInterval(delay || 0);
}

globalThis.clearInterval = Timer.clear;
globalThis.clearTimeout = Timer.clear;
