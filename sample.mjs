import path from 'std:path'
const { File } = Runtime.bind("fs");

// my cwd is inside build dir when I test PandJS
const filepath = path.join(Runtime.cwd(), "../sample.mjs");
const file = new File();
const promise = file.open(filepath)
promise.then((fd) => {
    console.log("fd:", fd);
    console.log(promise);
}).catch((err) => {
    console.log("error:", err.message);
});

console.log(promise);
