#include "sequential.h++"
#include <unistd.h>
#include "../variables.h++"

void sequential(){
  // no need to prefetch if last read is a hit
  if (hit != lastHit) {
    lastHit = hit;
    return;
  }

  // insert the block into the cache
  cache->insert(curr_offset+PAGE_SIZE);
}
