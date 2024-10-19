export function stringify(value, depth = 2, seen = new WeakSet()) {
  const nextDepth = depth - 1;

  if (value === null) return 'null';
  if (typeof value === 'undefined') return 'undefined';
  if (typeof value === 'boolean' || typeof value === 'number') return value.toString();
  if (typeof value === 'bigint') return value.toString() + 'n';
  if (typeof value === 'string') return `'${value}'`;
  if (typeof value === 'symbol') return value.toString();

  if (typeof value === 'function') {
    return `[Function ${value.name || '(anonymous)'}]`;
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

  if (value instanceof Set) {
    let entries = Array.from(value).map(v => stringify(v, nextDepth, seen));
    return `Set {${entries.join(', ')}}`;
  }

  if (typeof value === 'object') {
    if (seen.has(value)) return '[Circular]';
    seen.add(value);

    if (depth <= 0) {
      return `[Object${value.name ? `${ value.name}]` : "]"}`;
    }
    
    let keys = Object.keys(value);
    let keyValues = keys.map(key => `${key}: ${stringify(value[key], nextDepth, seen)}`);
    return `{ ${keyValues.join(', ')} }`;
  }

  return String(value);
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
