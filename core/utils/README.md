### Transcoding utils

Example usage of `hex` utility.

```cpp
#include "hex.h"
#include <iostream>
#include <memory>
#include <string>

int main() {
  size_t len = 14;
  const char *hex = "4d69636861c582";

  size_t out_len = hex::binary_length_from_hex(len);
  std::unique_ptr<char[]> out(new char[out_len]);

  if (hex::hex_to_binary(hex, len, out.get())) {
    std::string str(out.get(), out_len);
    std::cout << str << "\n";
  } else {
    std::cerr << "Invalid input\n";
  }
  
}
```
