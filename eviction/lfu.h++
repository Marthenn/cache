#ifndef CACHE_LFU_H
#define CACHE_LFU_H

#include "../variables.h++"

struct LFUCacheData : CacheData {
  int frequency;
};

struct LFUCacheStruct : BaseCacheStruct {
  std::list<LFUCacheData> cacheList;
  std::unordered_map<off64_t, typename std::list<LFUCacheData>::iterator> cacheMap;

  void insert(off64_t) override;
  void erase(off64_t) override;
  CacheData find(off64_t) override;
};

void lfu(off64_t offset);

#endif //CACHE_LFU_H
