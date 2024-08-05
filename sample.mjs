import { assert } from 'std:assert';

assert(2 + 2 == 4);
console.log("Hi");
console.log(import.meta.resolve('./'));
console.log(`Dirname: ${import.meta.dirname}`)
console.log(`Filename: ${import.meta.filename}`);
console.log(`Url: ${import.meta.url}`);
