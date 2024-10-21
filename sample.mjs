
try {
  // very edge case
  await import("./err.js");
} catch (err) {
  console.log("Handled dynamic import:", err.message);
}

const obj = {
  hello: "world",
  bool: true,
  nested: {
    nested2: {
      nested3: {
        nested4: {
          message: "hello world",
        }
      }
    }
  }
}

console.log("Hello world", 2323, new Date());
console.log(obj);
