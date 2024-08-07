## std:assert module

Assertions are built-in to help developers write tests.

```js
import {...} from 'std:assert';
```

### `assert(condition: boolean, message?: string)`

Throws `AssertionError` if condition is not true.

### `assertStrictEqual(actual: any, expected: any, message: message)`

Throws `AssertionError` if actual value is not strictly equal to the expected value.

### `assertThrows(func: function, errClass?: Error, message?: string)`

Throws `AssertionError` if specified function does not throw error.
