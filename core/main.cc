#include "pand.h"
#include <exception>

using namespace pand::core;

int main() {
  Pand *pand = Pand::get();
  try {
    pand->run("../sample.js");
  } catch (...) {
    pand->destroy();
    return 1;
  }

  pand->destroy();

  return 0;
}
