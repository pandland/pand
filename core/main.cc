#include "pand.h"

using namespace pand::core;

int main() {
  Pand *pand = Pand::get();
  try {
    pand->run("../sample.js");
    pand->exit(0);
  } catch (...) {
    pand->exit(1);
  }
  return 0;
}
