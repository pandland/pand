import { format } from 'std:inspect';

console.log = (...args) => { Runtime.print(format(args) + "\n"); }
console.error = (...args) => { Runtime.printerr(format(args) + "\n"); }

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

  return timer.setTimeout(delay);
}

globalThis.setInterval = (cb, delay, ...args) => {
  const timer = new Timer();
  timer.onTimeout = () => {
    cb(...args);
  }

  return timer.setInterval(delay);
}

globalThis.clearInterval = Timer.clear;
globalThis.clearTimeout = Timer.clear;
