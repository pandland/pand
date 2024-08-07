## Runtime object

Global object - it holds information about process and runtime.

### `Runtime.platform: string`

Returns current platform as a string, possible values:

  - `linux`
  - `win32`
  - `darwin`
  - `freebsd`
  - `openbsd`
  - `unix`
  - `unknown`

### `Runtime.pid: number`

Returns process id (number).

### `Runtime.version: string`

Returns runtime's version as a string - [semantic version](https://semver.org/) prefixed with `v` (for example `"v1.0.0"`)

### `Runtime.argv: string[]`

Returns the script arguments (as an array of strings) to the program.

```sh
done ./your-script.js arg1 arg2 ... argN
```

```js
Runtime.argv.forEach(item => {
  console.log(`Arg: ${item}`);
});

// output:
Arg: done
Arg: ./your-script.js
Arg: arg1
Arg arg2
...
Arg: argN
```

### `Runtime.cwd(): string`

Returns current working directory as a string.

### `Runtime.env(name: string): string | null`

Returns specified environment variable as a string, if not found - returns null.

### `Runtime.exit(status?: number)`

Forcefully exits process without waiting for pending operations.
