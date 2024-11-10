import path from 'std:path'
const { File, O_WRONLY, O_CREAT, O_APPEND } = Runtime.bind("fs");

async function handleFile() {
    const filepath = path.join(Runtime.cwd(), "../test.md");
    let file = new File();
    await file.open(filepath, O_WRONLY | O_CREAT | O_APPEND);

    const buf = Buffer.from("a");
    const size = await file.write(buf);

    await file.close();
}

for (let i = 0; i < 1000; ++i) {
    await handleFile();
}
