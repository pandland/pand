export function test1() {
  console.log("Test1 call");
  throw new Error("Test error");
  console.log(`Dirname: ${import.meta.dirname}`)
  console.log(`Filename: ${import.meta.filename}`);
  console.log(`Url: ${import.meta.url}`);
}
