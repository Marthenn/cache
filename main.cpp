#include "variables.h++"
#include "eviction/eviction.h++"
#include "prefetch/prefetch.h++"
#include <iostream>
#include <memory>
#include <fcntl.h>
#include <sstream>
#include <chrono>

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

  // initialize the buffer
  buffer = std::make_unique<char[]>(PAGE_SIZE*capacity);

  // open the file
  fd = open(mountingPoint, O_RDONLY);
  if (fd == -1) {
    std::cerr << "Error: could not open file " << mountingPoint << std::endl;
    return 1;
  }

  auto start_time = std::chrono::high_resolution_clock::now();

  // read the trace file line by line
  std::string line;
  while (std::getline(traceFile, line)) {
    std::istringstream iss(line);
    double timestamp;
    int deviceID, size, rw;
    off64_t offset;

    if (!(iss >> timestamp >> deviceID >> offset >> size >> rw)) {
      std::cerr << "Error: could not read line " << line << std::endl;
      return 1;
    }

    // only consider reads
    if (rw != 1) continue;

    // set the current offset
    curr_offset = offset;

    total++;
    evict(offset);
    prefetch();
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_time = end_time - start_time;
  std::cout << "Elapsed time: " << elapsed_time.count() << "s" << std::endl;
  std::cout << "Hit rate: " << (double)hit/(double)total << std::endl;

  return 0;
}
