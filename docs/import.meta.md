## `import.meta` object

You can use this object to obtain informations about module, like filename, dirname or URL.

### `import.meta.url: string`

Returns URL of the current module.

```js
// sample.js

console.log(import.meta.url);
/* output:
 * file:///home/michal/Workspace/hello/sample.js
 */
```

### `import.meta.filename: string`

Returns the absolute path to the current module (contains OS specific path separators).

### `import.meta.dirname: string`

Returns the absolute path to the parent directory of the module (contains OS specific path separators).

### `import.meta.resolve(specifier: string): string`

Resolve specifiers relative to the current module.
