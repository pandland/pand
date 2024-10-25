const sample = Buffer.from("sample:");
const result = Buffer.concat(sample, Buffer.from([65, 66, 66, 65]));

console.log(result);
console.log(result.toString());
console.log(JSON.stringify(result));

const buf1 = Buffer.from([65, 66, 66, 65]);
const buf2 = Buffer.from("ABBA");
console.log(buf1.equals(buf2));
