class Hi {}
const hi = () => {}
const plainObj = { id: 200, message: 'Hello"' }
const err = new Error("hello");
const typedArr = new Uint8Array(102);

const promise = new Promise((resolve, reject) => {
  setTimeout(() => {
    resolve(22);
    onTimeout();
  }, 1000);
});

function onTimeout() {
  console.log(promise);
}

//console.log(typedArr);
//console.log(err);
console.log(promise);
//console.log(plainObj);
//console.log(WebAssembly);
//console.log(Hi);
//console.log(hi);
