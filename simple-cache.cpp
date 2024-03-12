#include <unordered_map>
#include <list>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <stdexcept>

namespace cache {
  class Cache {
  private:
    const char *const mountingPoint;
    const int capacity = 0;
    std::list<std::pair<off64_t, char>> cacheList;
    std::unordered_map<off64_t, typename std::list<std::pair<off64_t, char>>::iterator> cacheMap;

    char readFromDisk(const off64_t offset) {
      int fd = open(mountingPoint, O_RDONLY);
      if (fd == -1) {
        perror("Error opening file");
        throw std::runtime_error("Error opening file");
      }

      char temp[1];
      ssize_t bytesRead = pread(fd, temp, 1, offset);
      if (bytesRead == -1) {
        perror("Error reading file");
        throw std::runtime_error("Error reading file");
      }
      close(fd);

      return temp[0];
    }

  public
    Cache() : capacity(0), mountingPoint("/dev/sda") {}
    Cache(int capacity, const char* mountingPoint) : capacity(capacity), mountingPoint(mountingPoint) {}

    char get(const off64_t offset) {
      if (cacheMap.find(offset) == cacheMap.end()) {
        if (cacheList.size() == capacity) {
          auto last = cacheList.back();
          cacheList.pop_back();
          cacheMap.erase(last.first);
        }
        cacheList.emplace_front(offset, readFromDisk(offset));
        cacheMap[offset] = cacheList.begin();
      } else {
        cacheList.splice(cacheList.begin(), cacheList, cacheMap[offset]);
      }
      return cacheMap[offset]->second;
    }
  };
}
