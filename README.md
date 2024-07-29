> ⚠️ Early stage of development.

# done.js

My own JavaScript runtime - currently, just randomly messing around with v8 engine in C++.

### TODO:

- [x] Try to bind SWC (written in Rust) into C++ my codebase
- [ ] Integrate my async IO library for runtime's event loop: [luxio](https://github.com/michaldziuba03/luxio).

I try to support TypeScript from the beginning - currently simple TS is stripped to JS (for example enums are not supported).

```ts
// simple TS code runs
function add(a: number, b: number): number {
  return a + b;
}

add(22, 11);
```
