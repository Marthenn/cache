#include <unordered_map>
#include <list>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <string>
#include <stdexcept>

namespace cache {
  struct Key {
    std::string mountingPoint;
    off_t offset;

    Key(std::string mountingPoint, off_t offset) : mountingPoint(std::move(mountingPoint)), offset(offset) {}
  };

  class Cache {
  private:
    int capacity = 0;
    std::list<std::pair<Key, char>> cacheList;
    std::unordered_map<Key, typename std::list<std::pair<Key, char>>::iterator> cacheMap;

    static char readFromDisk(const Key& key) {
      int fd = open(key.mountingPoint.c_str(), O_RDONLY);
      if (fd == -1) {
        perror("Error opening file");
        throw std::runtime_error("Error opening file");
      }

      char temp[1];
      ssize_t bytesRead = pread(fd, temp, 1, key.offset);
      if (bytesRead == -1) {
        perror("Error reading file");
        throw std::runtime_error("Error reading file");
      }
      close(fd);

      return temp[0];
    }

  public:
    char get(Key key) {
      if (cacheMap.find(key) == cacheMap.end()) {
        if (cacheList.size() == capacity) {
          auto last = cacheList.back();
          cacheList.pop_back();
          cacheMap.erase(last.first);
        }
        char value = readFromDisk(key);
        cacheList.emplace_front(key, value);
        cacheMap[key] = cacheList.begin();
        return value;
      } else {
        auto it = cacheMap[key];
        cacheList.splice(cacheList.begin(), cacheList, it);
        return it->second;
      }
    }
  };
}
