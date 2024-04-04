#include "sequential.h++"
#include <unistd.h>
#include "../variables.h++"

int lastHit = 0; // to detect if last read is a hit or miss

void sequential(){
  // no need to prefetch if last read is a hit
  if (hit != lastHit) {
    lastHit = hit;
    return;
  }

  // insert the block into the cache
  cache->insert(curr_offset+PAGE_SIZE);
}
