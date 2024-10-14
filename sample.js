const id = setTimeout((arg) => {
  console.log(`timeout callback with arg: ${arg}`);
}, 5000, 200);

console.log(`timeout id: ${id}`);

let no = 1;

setInterval(() => {
  console.log(`interval callback #${no}`);
  no++;
}, 1000);
