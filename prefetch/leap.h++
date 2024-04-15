#ifndef CACHE_LEAP_H
#define CACHE_LEAP_H

// based on https://www.usenix.org/system/files/atc20-maruf.pdf

#include "../variables.h++"
#include <iostream>
#include <vector>
#include <optional>

// use the size of 32 for now
// the paper stated that for small size like 32 it already gives a significant improvement
const int HISTORY_SIZE = 32;

// the number of split for the history size
// use 8 for the time being (the paper start with window size of 4)
const int nSplit = 8;

extern std::vector<off64_t> offsetsHistory;
extern std::optional<off64_t> major_trend;
extern unsigned long long lastPrefetchWindowSize;
const unsigned long long maxPrefetchWindowSize = capacity / PAGE_SIZE;

void leap();

#endif //CACHE_LEAP_H
