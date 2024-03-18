#include <unordered_map>
#include <list>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <stdexcept>
#include <vector>

int main() {
  const char *const mountingPoint = "/dev/sda";
  const int capacity = 50;
  std::list<std::pair<off64_t, char>> cacheList;
  std::unordered_map<off64_t, typename std::list<std::pair<off64_t, char>>::iterator> cacheMap;

  int hit = 0;
  std::vector<off64_t> offset;

  for (long x : offset) {
    if (cacheMap.find(x) == cacheMap.end()) {
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
      ssize_t bytesRead = pread(fd, temp, 1, x);
      if (bytesRead == -1) {
        perror("Error reading file");
        throw std::runtime_error("Error reading file");
      }
      close(fd);

      cacheList.emplace_front(x, temp[0]);
      cacheMap[x] = cacheList.begin();
    } else {
      cacheList.splice(cacheList.begin(), cacheList, cacheMap[x]);
      cacheMap[x] = cacheList.begin();
      hit++;
    }
  }

  return 0;
}