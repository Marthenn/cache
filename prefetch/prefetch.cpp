#include "prefetch.h++"
#include "../variables.h++"

#include "leap.h++"
#include "sequential.h++"

void initPrefetch(const std::string& algorithm) {
  if (algorithm == "leap") {
    prefetch = leap;
  } else if (algorithm == "sequential") {
    prefetch = sequential;
  }
  else {
    std::cerr << "Error: unknown prefetch algorithm " << algorithm << std::endl;
  }
}
