const id = setTimeout((arg) => {
  console.log(`timeout callback with arg: ${arg}`);
}, 5000, 200);

console.log(`timeout id: ${id}`);
