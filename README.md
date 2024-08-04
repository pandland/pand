> ⚠️ Early stage of development.

# done.js

My own JavaScript runtime - currently, just randomly messing around with v8 engine in C++. Very unstable.

Once I will figure out how things work together - I will rewrite this project.

### TODO:

- [x] Try to bind SWC (written in Rust) into my C++ codebase
- [x] Integrate my async IO library for runtime's event loop: [luxio](https://github.com/michaldziuba03/luxio).
- [x] Create timers
- [x] Clunky TCP support
- [ ] Basic support for ES6 imports

I try to support TypeScript from the beginning - currently every TS file is stripped to JS (and enums are not supported).

![image](https://github.com/user-attachments/assets/c1e97b8e-997b-4794-acbd-76f22924910f)

> Executing sample.ts code
