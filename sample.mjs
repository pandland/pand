import path from 'std:path'
const { File, O_RDONLY } = Runtime.bind("fs");

// my cwd is inside build dir when I test PandJS
const filepath = path.join(Runtime.cwd(), "../sample.mjs");
const file = new File();
await file.open(filepath, O_RDONLY)

const buf = new Buffer(1024);
const size = await file.read(buf);

console.log("size:", size);
console.log(buf);

await file.close();
