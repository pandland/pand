export function test1() {
  println("Test1 call");
  println(`Dirname: ${import.meta.dirname}`)
  println(`Filename: ${import.meta.filename}`);
  println(`Url: ${import.meta.url}`);
}
