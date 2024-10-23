'use strict';

const isPlainObject = obj => obj.constructor === Object && Object.getPrototypeOf(obj) === Object.prototype;

function isConstructor(func) {
  return typeof func === 'function' && !!func.prototype && func.prototype.constructor === func && func.toString().startsWith('class');
}

export function stringify(value, depth = 2, seen = new WeakSet()) {
  const nextDepth = depth - 1;

  if (value === null) return 'null';
  if (typeof value === 'undefined') return 'undefined';
  if (typeof value === 'boolean' || typeof value === 'number') return value.toString();
  if (typeof value === 'bigint') return value.toString() + 'n';
  if (typeof value === 'string') return JSON.stringify(value);  // it's better than nothing
  if (typeof value === 'symbol') return value.toString();

  if (typeof value === 'function') {
    const typename = isConstructor(value) ? "Class" : "Function:";
    return `[${typename} ${value.name || '(anonymous)'}]`;
  }

  if (value instanceof Date) {
    return value.toISOString();
  }

  if (Array.isArray(value)) {
    return `[ ${value.map(v => stringify(v, nextDepth, seen)).join(', ')} ]`;
  }

  if (value instanceof Map) {
    let entries = [];
    value.forEach((val, key) => {
      entries.push(`${stringify(key, nextDepth, seen)} => ${stringify(val, nextDepth, seen)}`);
    });
    return `Map {${entries.join(', ')}}`;
  }

  if (value instanceof Error) {
    return value.stack || `${value.name}: ${value.message}`;
  }

  if (value instanceof Promise) {
    return `Promise { <${Runtime.promiseState(value)}> }`;
  }

  if (value instanceof ArrayBuffer) {
    const buf = new Uint8Array(value);
    return `ArrayBuffer(${value.byteLength}) ${stringifyByteArray(buf, value.byteLength)}`;
  }

  if (value instanceof SharedArrayBuffer) {
    const buf = new Uint8Array(value);
    return `SharedArrayBuffer(${value.byteLength}) ${stringifyByteArray(buf, value.byteLength)}`;
  }

  if (value instanceof DataView) {
    const buf = new Uint8Array(value.buffer);
    return `DataView(${value.byteLength}) ${stringifyByteArray(buf, value.byteLength)}`;
  }

  if (ArrayBuffer.isView(value)) {
    return inspectTypedArray(value);
  }

  if (value instanceof RegExp) {
    return String(value);
  }

  if (value instanceof Set) {
    let entries = Array.from(value).map(v => stringify(v, nextDepth, seen));
    return `Set {${entries.join(', ')}}`;
  }

  if (typeof value === 'object') {
    if (seen.has(value)) return '[Circular]';
    seen.add(value);

    let str = "";
    if (value.constructor && !isPlainObject(value) && value.constructor.name) {
      str += `[${value.constructor.name}] `
    } else if (!Runtime.isProxy(value) && value[Symbol.toStringTag]) {
      str += `Object [${value[Symbol.toStringTag]}] `;
    }

    if (depth <= 0) {
      return `[Object ${value.name ? `${ value.name}]` : "]"}`;
    }
    
    let keys = Object.keys(value);
    let keyValues = keys.map(key => `${key}: ${stringify(value[key], nextDepth, seen)}`);
    const content = keyValues.join(', ');
    return str + "{" + (content.length > 0 ? ` ${content} }` : "}");
  }

  return String(value);
}

function stringifyByteArray(buff, totalLength, limit = 100) {
  const rowLimit = 12;
  const multiline = totalLength > rowLimit;
  const visibleItems = Array.from(buff.slice(0, limit));
  const numRows = Math.ceil(visibleItems.length / rowLimit);
  if (totalLength === 0) {
    return "[]";
  }

  let str = `[${multiline ? "\n " : ""} `

  for (let row = 0; row < numRows; row++) {
    const start = row * Math.ceil(visibleItems.length / numRows);
    const end = start + Math.ceil(visibleItems.length / numRows);

    str += visibleItems.slice(start, end).join(', ');

    if (row < numRows - 1) {
      str += ',\n  ';
    }
  }

  if (totalLength > limit) {
    const remainingItems = totalLength - limit;
    str += `,\n  ... ${remainingItems} more item${remainingItems != 1 ? 's' : ''}`;
  }

  str += `${multiline ? "\n" : " "}]`;
  return str;
}

function inspectTypedArray(buff, limit = 100) {
  const typeName = buff.constructor.name || 'TypedArray';
  const totalLength = buff.length;

  let str = `${typeName}(${totalLength}) ${stringifyByteArray(buff, totalLength)}`;
  return str;
}

export function format(args, maxDepth) {
  let str = ""

  for (let item of args) {
    if (typeof item == "string") {
      str += item;
    } else {
      str += stringify(item, maxDepth);
    }
    str += " ";
  }

  return str;
}
