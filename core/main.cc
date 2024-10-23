#include "errors.h"
#include "pand.h"
#include <iostream>
#include <new>
#include <stdexcept>

using namespace pand::core;

// in the future we will make pand::cli
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "PandJS: Simple, free and open-source JavaScript runtime "
                 "environment.\n\n";
    std::cout << "Website: " << "https://pandland.github.io\n";
    std::cout << "GitHub: " << "https://github.com/pandland/pand\n";
    std::cout << "\nTo execute script run:\n\n"
              << "  pand [ script.js ] [arguments]\n\n";
    return 0;
  }

  // init Pand singleton
  Pand *pand = Pand::get();
  try {
    const char *entryfile = argv[1];
    pand->run(entryfile, argc, argv);
  } catch (const CriticalException &err) {
    return 1;
  } catch (const std::bad_alloc &err) {
    // this exception is likely to happen for some users.
    Errors::printError("Bad alloc");
  } catch (const std::exception &err) {
    Errors::printError(err.what());
    return 1;
  } catch (...) {
    Errors::printError("Unknown critical error");
    return 1;
  }

  return 0;
}
