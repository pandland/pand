const buf = new ArrayBuffer(8000);
const arr = new Array(10000);

const obj = {
  a: arr
}

arr[33] = 2

obj.ref = obj;
arr[43] = obj;

const set = new Set([1, 2, 3]);
set.add(4);
const promise = new Promise(() => {});
console.log(set);
