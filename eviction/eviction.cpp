#include "eviction.h++"
#include "../variables.h++"

#include "lru.h++"
#include "lfu.h++"

void initEviction(const std::string& algorithm) {
  if (algorithm == "LRU") {
    cache = new LRUCacheStruct();
    evict = lru;
  } else if (algorithm == "LFU") {
    cache = new LFUCacheStruct();
    evict = lfu;
  }
  else {
    std::cerr << "Error: unknown eviction algorithm " << algorithm << std::endl;
  }
}
