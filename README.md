> ⚠️ Early stage of development.

# done.js

My own JavaScript runtime - currently, just randomly messing around with v8 engine in C++. Very unstable.

Once I will figure out how things work together - I will rewrite this project.

### TODO:

- [x] Try to bind SWC (written in Rust) into my C++ codebase
- [x] Integrate my async IO library for runtime's event loop: [luxio](https://github.com/michaldziuba03/luxio).
- [x] Create timers
- [x] Clunky TCP support
- [x] Basic support for ES6 imports

> Current state example:

```js
import { assert } from 'std:assert';

assert(2 + 2 == 4);
console.log("Hi");
console.log(import.meta.resolve('./'));
console.log(`Dirname: ${import.meta.dirname}`)
console.log(`Filename: ${import.meta.filename}`);
console.log(`Url: ${import.meta.url}`);
```

