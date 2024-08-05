const { API_KEY } = await import('./empty.mjs');

println(import.meta.resolve('../'));
println(`Dirname: ${import.meta.dirname}`)
println(`Filename: ${import.meta.filename}`);
println(`Url: ${import.meta.url}`);
