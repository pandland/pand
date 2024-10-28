const chars = "0123456789abcdef"

function hexify(byte) {
  const upper = (byte & 0xf0) >> 4;
  const lower = byte & 0xf;

  return chars[upper] + chars[lower];
}

export function uuidv4() {
  const bytes = Buffer.random(16);

  bytes[6] = (bytes[6] & 0x0f) | 0x40; // set "4" in upper bytes
  bytes[8] = (bytes[8] & 0x3f) | 0x80;

  return (
    hexify(bytes[0]) +
    hexify(bytes[1]) +
    hexify(bytes[2]) +
    hexify(bytes[3]) +
    "-" +
    hexify(bytes[4]) +
    hexify(bytes[5]) +
    "-" +
    hexify(bytes[6]) +
    hexify(bytes[7]) +
    "-" +
    hexify(bytes[8]) +
    hexify(bytes[9]) +
    "-" +
    hexify(bytes[10]) +
    hexify(bytes[11]) +
    hexify(bytes[12]) +
    hexify(bytes[13]) +
    hexify(bytes[14]) +
    hexify(bytes[15])
  );
}
