#include "pand.h"
#include <iostream>

using namespace pand::core;

// in the future we will make pand::cli
int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cout << "Simple, free and open-source JavaScript runtime environment.\n\n";
    std::cout << "Website: " << "https://pandland.github.io\n";
    std::cout << "GitHub: " << "https://github.com/pandland/pand\n";
    std::cout << "\nTo execute script run:\n\n" << "  pand [FILEPATH]\n\n";
    return 0;
  }

  // init Pand singleton
  Pand *pand = Pand::get();
  try {
    const char *entryfile = argv[1];
    pand->run(entryfile, argc, argv);
  } catch (...) {
    pand->destroy();
    return 1;
  }

  pand->destroy();

  return 0;
}
