const id = setInterval((arg) => {
  console.log(`Interval callback with arg: ${arg}`);
}, 500, 40);

setTimeout(() => {
  clearInterval(id);
  console.log('Clear interval')
}, 5000);
