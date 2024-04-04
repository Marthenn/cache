#include "variables.h++"
#include "eviction/eviction.h++"
#include "prefetch/prefetch.h++"
#include <iostream>
#include <memory>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  if (argc < 6) {
    std::cerr << "Usage: " << argv[0] << " <mounting point> <trace file> <eviction algorithm> <prefetch algorithm> <cache capacity>" << std::endl;
  }

  // setting the command line arguments
  mountingPoint = argv[1];
  traceFile.open(argv[2]);
  if (!traceFile.is_open()) {
    std::cerr << "Error: could not open file " << argv[2] << std::endl;
    return 1;
  }
  initEviction(argv[3]);
  initPrefetch(argv[4]);
  capacity = std::stol(argv[5]);

  return 0;
}
