async function canThrow() {
  throw new Error('danger');
}

await canThrow().catch((err) => {
  console.log("we got error!");
});
console.log("heheszq")