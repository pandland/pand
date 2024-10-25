const sample = Buffer.from("sample:");
console.log(
  Buffer.concat(sample, Buffer.from([65, 66, 66, 65]), Buffer.random(8))
);
