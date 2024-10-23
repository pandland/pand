class Hi {}
const hi = () => {}
const plainObj = { id: 200, message: 'Hello"' };
plainObj.ref = plainObj;

const err = new Error("hello");
const typedArr = new Uint8Array(102);

const re = /ab+c/;
//const re = new RegExp("ab+c");

const promise = new Promise((resolve, reject) => {
  setTimeout(() => {
    resolve(22);
    onTimeout();
  }, 1000);
});

function onTimeout() {
  console.log(promise);
}

const buf = new ArrayBuffer(11);
//console.log(buf);

function* myGenerator() {}

console.log(re);

const target = {};
const handler = {
  get: (obj, prop) => `Intercepted ${prop}`
};
const proxy = new Proxy(target, handler);
///console.log(proxy);  // Proxy {}

//console.log(myGenerator);
//console.log(NaN);
//console.log(typedArr);
//console.log(err);
//console.log(promise);
//console.log(plainObj);
//console.log(WebAssembly);
//console.log(Hi);
//console.log(hi);
