const chars = "0123456789abcdef"

function hex(byte) {
  const upper = (byte & 0xf0) >> 4;
  const lower = byte & 0xf;

  return chars[upper] + chars[lower];
}

export function uuidv4() {
  const bytes = Buffer.random(16);

  bytes[6] = (bytes[6] & 0x0f) | 0x40;
  bytes[8] = (bytes[8] & 0x3f) | 0x80;

  return (
    hex(bytes[0]) +
    hex(bytes[1]) +
    hex(bytes[2]) +
    hex(bytes[3]) +
    "-" +
    hex(bytes[4]) +
    hex(bytes[5]) +
    "-" +
    hex(bytes[6]) +
    hex(bytes[7]) +
    "-" +
    hex(bytes[8]) +
    hex(bytes[9]) +
    "-" +
    hex(bytes[10]) +
    hex(bytes[11]) +
    hex(bytes[12]) +
    hex(bytes[13]) +
    hex(bytes[14]) +
    hex(bytes[15])
  );
}
