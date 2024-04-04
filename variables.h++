#ifndef CACHE_VARIABLES_H
#define CACHE_VARIABLES_H

#include <list>
#include <unordered_map>
#include <memory>
#include <fstream>

// function pointer for eviction and prefetch
void (*evict)(off64_t);
void (*prefetch)();

// hit rate variables
int hit = 0;
int total = 0;
off64_t curr_offset;

// file variables
int fd;
const char* mountingPoint;
std::ifstream traceFile;

// buffer variables
std::unique_ptr<char[]> buffer;
long capacity;

// system variables
// note: change this if the block size is different
//       you can get the block size of your system by running `getconf PAGE_SIZE`
const ssize_t PAGE_SIZE = 4096;

// cache struct
// note: the use of std::list is to allow for O(1) removal of elements
//       and the use of std::unordered_map is to allow for O(1) search of elements
//       the use of std::list::iterator is to allow for modularity of the CacheData type
//       BaseCacheStruct is used to allow for the cache to be changed to a different type
struct CacheData {
  char data[PAGE_SIZE];
  off64_t offset;
};
struct BaseCacheStruct {
  virtual ~BaseCacheStruct() = default;
  virtual void insert(off64_t) = 0;
  virtual void erase(off64_t) = 0;
  virtual CacheData find(off64_t) = 0;
};
BaseCacheStruct* cache;

#endif //CACHE_VARIABLES_H
