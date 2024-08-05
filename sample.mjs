import { assert, assertStrictEqual, assertThrows } from 'std:assert';

function willThrow() {
  throw new Error("Some error");
}

assertThrows(willThrow);
assert(2 + 2 == 4);
assertStrictEqual(2, 2);
console.log(`API_KEY is ${Runtime.env('API_KEY')}`)
console.log("Hi");
console.log(import.meta.resolve('./'));
console.log(`Dirname: ${import.meta.dirname}`)
console.log(`Filename: ${import.meta.filename}`);
console.log(`Url: ${import.meta.url}`);
