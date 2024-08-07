## Modules

Available built-in modules:

- `std:assert`
- `std:net`

### Only ES6 module syntax is supported for importing modules.

#### Do:

```js
import { sayHello } from './my-module.js';
```

#### Don't:

```js
const { sayHello } = require('./my-module.js');
```

> I do not plan to support CommonJS modules.


### Dynamic loading is supported

```js
await import('std:assert');
```
