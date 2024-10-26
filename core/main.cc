#include "errors.h"
#include "pand.h"
#include <cstdlib>
#include <iostream>

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
    return EXIT_SUCCESS;
  }

  Pand *pand = Pand::get();
  try {
    const char *entryfile = argv[1];
    pand->run(entryfile, argc, argv);
  } catch (const CriticalException &err) {
    return EXIT_FAILURE;
  } catch (const std::bad_alloc &err) {
    // this exception is likely to happen for some users.
    Errors::printError("Bad alloc");
  } catch (const std::exception &err) {
    Errors::printError(err.what());
    return EXIT_FAILURE;
  } catch (...) {
    Errors::printError("Unknown critical error");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
