#include <memory>
#include <fstream>
#include <list>
#include <unordered_map>

// hit rate variables
int hit = 0;
int total = 0;

// file variables
int fd;
const char* mountingPoint;
std::ifstream traceFile;

// buffer variables
std::unique_ptr<char[]> buffer;
long capacity;

// system variables
ssize_t BLOCK_SIZE;

// cache struct
// note: the use of std::list is to allow for O(1) removal of elements
//       and the use of std::unordered_map is to allow for O(1) search of elements
//       the use of std::list::iterator is to allow for modularity of the cacheData type
template <typename T>
struct cache{
  std::list<T> cacheList;
  std::unordered_map<off64_t, typename std::list<T>::iterator> cacheMap;
};
