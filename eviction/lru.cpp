#include "lru.h++"
#include <algorithm>
#include <unistd.h>

void LRUCacheStruct::insert(off64_t offset) {
  ssize_t bytesRead = pread(fd, buffer.get(), PAGE_SIZE, offset);
  if (bytesRead == -1) {
    throw std::runtime_error("pread failed in insert");
  }

  // if the cache is full, evict the least recently used element
  if (cacheList.size() == capacity) {
    cacheMap.erase(cacheList.back().offset);
  }

  // insert the new element
  CacheData newElement;
  std::copy(buffer.get(), buffer.get() + PAGE_SIZE, newElement.data);
  newElement.offset = offset;
  cacheList.push_front(newElement);
  cacheMap[offset] = cacheList.begin();
}

void LRUCacheStruct::erase(off64_t offset) {
  auto it = cacheMap.find(offset);
  if (it == cacheMap.end()) {
    return;
  }
  cacheList.erase(it->second);
  cacheMap.erase(it);
}

CacheData LRUCacheStruct::find(off64_t offset) {
  auto it = cacheMap.find(offset);
  if (it == cacheMap.end()) {
    CacheData empty;
    empty.offset = -1;
    return empty;
  }
  return *it->second;
}

void lru(off64_t offset) {
  // if hit, copy to buffer
  auto data = cache->find(offset);
  if (data.offset != -1) {
    std::copy(data.data, data.data + PAGE_SIZE, buffer.get());
    hit++;
    return;
  }

  // if miss, insert to cache
  cache->insert(offset);
}
