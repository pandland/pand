let no = 1;

const intervalId = setInterval(() => {
  console.log(`interval callback #${no}`);
  no++;
}, 500);

console.log(`Interval id: ${intervalId}`);

const id = setTimeout((arg) => {
  console.log(`timeout callback with arg: ${arg}`);
  clearInterval(intervalId);
  throw new Error("error during timeout :o");
}, 4000, 200);

console.log(`timeout id: ${id}`);
