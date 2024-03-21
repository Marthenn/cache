#include <unordered_map>
#include <list>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

struct {
  int frequency;
  char data;
} typedef Cache;

int main() {
  const char *const mountingPoint = "/dev/sda";
  const long capacity = 100; // capacity of the cache in bytes (currently)
  char temp[1]; // temporary buffer for reading bytes from the disk
  std::list<Cache> cacheList; // the list of cached bytes
  std::unordered_map<off64_t, typename std::list<Cache>::iterator> cacheMap; // point an offset to the corresponding byte in the cache
  // NOTE: the cacheMap is a map of iterators, not values to allow flexibility in the cacheList later on

  // hit count and total I/O operations
  int hit = 0;
  int total = 0;

  // try to open the trace file
  std::ifstream traceFile("msr.cut.per_10k.rw_40_60.1370.trace");
  if (!traceFile.is_open()) {
    throw std::runtime_error("Error opening trace file");
  }

  // access the mounting point
  int fd = open(mountingPoint, O_RDONLY);
  if (fd == -1) {
    perror("Error opening file");
    throw std::runtime_error("Error opening file");
  }

  // read the trace file line by line
  std::string line;
  while (std::getline(traceFile, line)) {
    std::istringstream iss(line);
    double timestamp;
    int deviceId, size, rw;
    off64_t offset;

    // dissect the line into the variables
    if (!(iss >> timestamp >> deviceId >> offset >> size >> rw)) {
      throw std::runtime_error("Error parsing trace file");
    }

    // only consider read operations (rw == 1)
    if (rw != 1) {
      continue;
    }

    for(int i = 0; i < size; i++){
      // if the byte is not in the cache, read it from the disk and add it to the cache
      if (cacheMap.find(offset+i) == cacheMap.end()) {
        // if the cache is full, remove the least frequently used elemen
        if (cacheMap.size() == capacity) {
          auto last = std::min_element(cacheList.begin(), cacheList.end(), [](const Cache& a, const Cache& b) {
            return a.frequency < b.frequency;
          });
          cacheMap.erase(last->frequency);
          cacheList.erase(last);
        }

        // read the byte from the disk
        ssize_t bytesRead = pread(fd, temp, 1, offset+i);
        if (bytesRead == -1) {
          perror("Error reading file");
          throw std::runtime_error("Error reading file");
        }

        // add the cached data to the cacheList
        cacheList.emplace_front(Cache{1, temp[0]});
        cacheMap[offset+i] = cacheList.begin();
      } else {
        // if the byte is in the cache, add its frequency
        cacheMap[offset+i]->frequency++;
        hit++;
      }
      total++;
    }
  }

  // close the access to the mounting point
  close(fd);

  // calculating the hit ratio
  double hitRatio = double(hit) / double(total);
  std::cout << "LFU Hit Ratio: " << hitRatio << std::endl;

  return 0;
}
