#include "pand.h"

using namespace pand::core;

int main() {
  Pand *pand = Pand::get();
  pand->run("../sample.js");

  return 0;
}
