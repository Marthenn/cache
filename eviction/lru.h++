#ifndef CACHE_LRU_H
#define CACHE_LRU_H

#include <cstdio>
#include "../variables.h++"

struct LRUCacheStruct: BaseCacheStruct {
  std::list<CacheData> cacheList;
  std::unordered_map<off64_t, typename std::list<CacheData>::iterator> cacheMap;

  void insert(off64_t) override;
  void erase(off64_t) override;
  CacheData find(off64_t) override;
};

void lru(off64_t offset);

#endif //CACHE_LRU_H
