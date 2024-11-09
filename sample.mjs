import path from 'std:path'
const { File, O_RDONLY, O_WRONLY, O_CREAT } = Runtime.bind("fs");

// my cwd is inside build dir when I test PandJS
const filepath = path.join(Runtime.cwd(), "../test.md");
const file = new File();
await file.open(filepath, O_WRONLY | O_CREAT)

const buf = Buffer.from("## Hello from fs.write");
const size = await file.write(buf);

console.log("size:", size);
console.log(buf);

await file.close();
