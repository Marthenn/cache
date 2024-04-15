#include "variables.h++"

void (*evict)(off64_t);
void (*prefetch)();
unsigned long long hit = 0;
unsigned long long lastHit = 0;
unsigned long long total = 0;
off64_t curr_offset;
int fd;
const char* mountingPoint;
std::ifstream traceFile;
std::unique_ptr<char[]> buffer;
unsigned long long capacity;
BaseCacheStruct* cache;
