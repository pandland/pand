/* Copyright (c) Michał Dziuba
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

"use strict";

/**
 * @typedef {Object} FmtContext
 * @property {WeakSet} seen
 * @property {number} maxArrayLength
 * @property {number} maxStringLength
 */

/**
 * @template T
 * @typedef {(value: T, depth: number, ctx: FmtContext) => string} Formatter
 */

const kDefaultDepth = 3;
const kMaxArrayLength = 100;

export function inspectMany(values, options) {
  let str = ""

  for (let value of values) {
    str += `${inspect(value, options)} `;
  }

  return str;
}

export function inspect(value, options) {
  const depth = options.depth || kDefaultDepth;
  const ctx = {
    seen: new WeakSet(),
    maxArrayLength: options.maxArrayLength || kMaxArrayLength,
  };

  if (typeof value === "string") {
    return value; // return string as is in the first depth.
  }

  return format(value, depth, ctx);
}

/** @type {Formatter<any>} */
function format(value, depth, ctx) {
  const seen = ctx.seen;
  const nextDepth = depth - 1;

  // detect primitives
  if (value === null) return "null";
  if (typeof value === "undefined") return "undefined";
  if (typeof value === "boolean" || typeof value === "number")
    return value.toString();
  if (typeof value === "bigint") return value.toString() + "n";
  if (typeof value === "string") return JSON.stringify(value); // it's better than nothing
  if (typeof value === "symbol") return value.toString();

  if (typeof value === "function") {
    const typename = isConstructor(value) ? "Class" : "Function:";
    return `[${typename} ${value.name || "(anonymous)"}]`;
  }

  if (value instanceof RegExp) {
    return String(value);
  }

  if (value instanceof WeakMap) {
    return "WeakMap {}";
  }

  if (value instanceof WeakSet) {
    return "WeakSet {}";
  }

  if (value instanceof Date) {
    return value.toISOString();
  }

  let isObject = false;
  let formatter;
  let typename;
  let prefix;

  if (typeof value === "object") {
    if (seen.has(value)) return "[Circular]";
    seen.add(value);
    isObject = true;
  }

  if (isArray(value)) {
    typename = "Array";
    formatter = formatArray;
  } else if (isPromise(value)) {
    typename = "Promise";
    prefix = typename;
    formatter = formatPromise;
  } else if (isMap(value)) {
    typename = "Map";
    prefix = iterablePrefix(typename, value.size);
    formatter = formatMap;
  } else if (isSet(value)) {
    typename = "Set";
    prefix = iterablePrefix(typename, value.size);
    format = formatSet;
  } else if (isError(value)) {
    typename = "Error";
    formatter = formatError;
  } else if (isArrayBuffer(value)) {
    typename = "ArrayBuffer";
    prefix = iterablePrefix(typename, value.byteLength);
    formatter = formatArrayBuffer;
  } else if (isSharedArrayBuffer(value)) {
    typename = "SharedArrayBuffer";
    prefix = iterablePrefix(typename, value.byteLength);
    formatter = formatArrayBuffer;
  } else if (isDataView(value)) {
    typename = "DataView";
    prefix = iterablePrefix(typename, value.byteLength);
  } else if (isTypedArray(value)) {
    typename = value.constructor.name || "TypedArray";
    prefix = iterablePrefix(typename, value.length);
    formatter = formatTypedArray;
  } else if (isObject) {
    if (value.constructor && !isPlainObject(value) && value.constructor.name) {
      typename = value.constructor.name;
      prefix = `[${typename}]`;
    } else if (isNamedObject(value)) {
      typename = `Object [${value[Symbol.toStringTag]}]`;
      prefix = typename;
    } else typename = "Object";
    formatter = formatObject;
  }

  if (depth <= 0) {
    return `[${typename || "Value"}]`;
  }

  if (prefix && formatter) {
    return `${prefix} ${formatter(value, nextDepth, ctx)}`;
  }

  if (prefix) {
    return prefix;
  }

  if (formatter) {
    return formatter(value, nextDepth, ctx);
  }

  return String(value);
}

/** @type {Formatter<Array>} */
function formatArray(arr, depth, ctx) {
  if (arr.length === 0) {
    return "[]";
  }

  const len = Math.min(arr.length, ctx.maxArrayLength);
  const hasMore = arr.length > len;
  const output = [];
  let empty = 0;

  for (let i = 0; i < len; ++i) {
    if (!Object.hasOwn(arr, i)) {
      empty++;
    } else if (empty) {
      output.push(`<${empty} empty item${empty != 1 ? "s" : ""}>`);
      output.push(format(arr[i], depth, ctx));
      empty = 0;
    } else {
      output.push(format(arr[i], depth, ctx));
    }
  }

  if (hasMore) {
    const remaining = arr.length - len;
    output.push(`<${remaining + empty} more items>`);
  } else if (empty) {
    output.push(`<${empty} empty items>`);
  }

  return `[ ${output.join(", ")} ]`;
}

function iterablePrefix(typename, size) {
  return `${typename}(${size})`;
}

/** @type {Formatter<Promise>} */
function formatPromise(value, ctx) {
  // need to call native C++ code
  const state = Runtime.promiseState(value);
  return `{ <${state}> }`;
}

/** @type {Formatter<Map>} */
function formatMap(value, depth, ctx) {
  if (value.size === 0) return "{}";
  let entries = [];
  value.forEach((val, key) => {
    entries.push(`${format(key, depth, ctx)} => ${format(val, depth, ctx)}`);
  });
  return `{ ${entries.join(", ")} }`;
}

/** @type {Formatter<Error>} */
function formatError(value, depth, ctx) {
  return value.stack || `${value.name}: ${value.message}`;
}

/** @type {Formatter<ArrayBuffer>} */
function formatArrayBuffer(value, depth, ctx) {
  const buf = new Uint8Array(value);
  return formatTypedArray(buf, depth, ctx);
}

/** @type {Formatter<Set>} */
function formatSet(value, depth, ctx) {
  let entries = Array.from(value)
    .map((v) => format(v, depth, ctx))
    .join(", ");
  return value.size > 0 ? `{ ${entries} }` : "{}";
}

/** @type {Formatter<Uint8Array>} */
function formatTypedArray(value, depth, ctx) {
  const limit = ctx.maxArrayLength;
  const totalLength = value.length;
  const rowLimit = 12;
  const multiline = totalLength > rowLimit;
  const visibleItems = Array.from(value.slice(0, limit));
  const numRows = Math.ceil(visibleItems.length / rowLimit);

  if (totalLength === 0) {
    return "[]";
  }

  let str = `[${multiline ? "\n " : ""} `;

  for (let row = 0; row < numRows; row++) {
    const start = row * Math.ceil(visibleItems.length / numRows);
    const end = start + Math.ceil(visibleItems.length / numRows);

    str += visibleItems.slice(start, end).join(", ");

    if (row < numRows - 1) {
      str += ",\n  ";
    }
  }

  if (totalLength > limit) {
    const remainingItems = totalLength - limit;
    str += `,\n  ... ${remainingItems} more item${
      remainingItems != 1 ? "s" : ""
    }`;
  }

  str += `${multiline ? "\n" : " "}]`;
  return str;
}

/** @type {Formatter<Object>} */
function formatObject(value, depth, ctx) {
  let keys = Object.keys(value);
  if (keys.length === 0)
    return "{}";

  let keyValues = keys.map((key) => {
    const formattedKey = format(key, depth, ctx);
    const formattedVal = format(value[key], depth, ctx);
    return `${formattedKey}: ${formattedVal}`;
  });
  const content = keyValues.join(", ");
  return `{ ${content} }`;
}

/* TYPE CHECKERS: */
function isArray(value) {
  return Array.isArray(value);
}

function isPromise(value) {
  return value instanceof Promise;
}

function isMap(value) {
  return value instanceof Map;
}

function isSet(value) {
  return value instanceof Set;
}

function isError(value) {
  return value instanceof Error;
}

function isArrayBuffer(value) {
  return value instanceof ArrayBuffer;
}

function isSharedArrayBuffer(value) {
  return value instanceof SharedArrayBuffer;
}

function isDataView(value) {
  return value instanceof DataView;
}

function isTypedArray(value) {
  return ArrayBuffer.isView(value) && !(value instanceof DataView);
}

function isConstructor(func) {
  return (
    typeof func === "function" &&
    !!func.prototype &&
    func.prototype.constructor === func &&
    func.toString().startsWith("class")
  );
}

function isNamedObject(obj) {
  // make sure it is not Proxy, because we do not want to call trap functions
  return !Runtime.isProxy(obj) && obj[Symbol.toStringTag];
}

// check if object named before assuming that it is plain object
function isPlainObject(obj) {
  return (
    obj.constructor === Object &&
    Object.getPrototypeOf(obj) === Object.prototype
  );
}
