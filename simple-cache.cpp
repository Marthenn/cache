#include <unordered_map>
#include <list>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

int main() {
  const char *const mountingPoint = "/dev/sda";
  const int capacity = 1000000;
  std::list<std::pair<off64_t, char>> cacheList;
  std::unordered_map<off64_t, typename std::list<std::pair<off64_t, char>>::iterator> cacheMap;

  int hit = 0;
  int total = 0;

  std::ifstream traceFile("msr.cut.per_10k.rw_40_60.1370.trace");
  if (!traceFile.is_open()) {
    throw std::runtime_error("Error opening trace file");
  }

  std::string line;
  while (std::getline(traceFile, line)) {
    std::istringstream iss(line);
    double timestamp;
    int deviceId, size, rw;
    off64_t offset;

    if (!(iss >> timestamp >> deviceId >> offset >> size >> rw)) {
      throw std::runtime_error("Error parsing trace file");
    }

    if (rw != 1) {
      continue;
    }

    for(int i = 0; i < size; i++){
      if (cacheMap.find(offset+i) == cacheMap.end()) {
        if (cacheMap.size() == capacity) {
          auto last = cacheList.back();
          cacheList.pop_back();
          cacheMap.erase(last.first);
        }
        char temp[1];
        int fd = open(mountingPoint, O_RDONLY);
        if (fd == -1) {
          perror("Error opening file");
          throw std::runtime_error("Error opening file");
        }
        ssize_t bytesRead = pread(fd, temp, 1, offset+i);
        if (bytesRead == -1) {
          perror("Error reading file");
          throw std::runtime_error("Error reading file");
        }
        close(fd);

        cacheList.emplace_front(offset+i, temp[0]);
        cacheMap[offset+i] = cacheList.begin();
      } else {
        cacheList.splice(cacheList.begin(), cacheList, cacheMap[offset+i]);
        cacheMap[offset+i] = cacheList.begin();
        hit++;
      }
      total++;
    }
  }

  double hitRatio = double(hit) / double(total);
  std::cout << "Hit Ratio: " << hitRatio << std::endl;

  return 0;
}