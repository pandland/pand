> ⚠️ Early stage of development.

# done.js

My own JavaScript runtime - currently, just randomly messing around with v8 engine in C++.

### TODO:

- [x] Try to bind SWC (written in Rust) into my C++ codebase
- [x] Integrate my async IO library for runtime's event loop: [luxio](https://github.com/michaldziuba03/luxio).
- [x] Create timers

I try to support TypeScript from the beginning - currently every TS file is stripped to JS (and enums are not supported).

![image](https://github.com/user-attachments/assets/dbb6b75b-ad1f-444c-a8a7-6482366eb403)

> Executing sample.ts code
