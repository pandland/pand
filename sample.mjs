const sample = Buffer.from("sample:");
const result = Buffer.concat(sample, Buffer.from([65, 66, 66, 65]));

console.log(Buffer.concat());
console.log(result);
console.log(result.toString());
console.log(JSON.stringify(result));
