#include "leap.h++"

#include <unordered_map>
#include <cmath>

std::vector<off64_t> offsetsHistory(HISTORY_SIZE);
std::optional<off64_t> major_trend = std::nullopt;
unsigned long long lastPrefetchWindowSize = 0;

std::optional<off64_t> boyer_moore(const std::vector<off64_t>& offsets, int w) {
  off64_t candidate = 0;
  int count = 0;

  for (int i = 0; i < w; i++) {
    if (count == 0) {
      candidate = offsets[i];
      count = 1;
    } else if (candidate == offsets[i]) {
      count++;
    } else {
      count--;
    }
  }

  count = 0;
  for (int i = 0; i < w; i++) {
    if (offsets[i] == candidate) {
      count++;
    }
  }

  if (count > w / 2) {
    return candidate;
  } else {
    return std::nullopt;
  }
}

std::optional<off64_t> findTrend() {
  int w = HISTORY_SIZE / nSplit;
  std::optional<off64_t> delta_maj;

  do {
    delta_maj = boyer_moore(offsetsHistory, w);
    w *= 2;
    if (delta_maj.has_value()) { // checking for safety
      if (!major_trend.has_value()) {
        major_trend = delta_maj;
      } else if (delta_maj.value() != major_trend.value()) {
        delta_maj = std::nullopt;
      }
    }
  } while(w <= HISTORY_SIZE && !delta_maj.has_value());

  return delta_maj;
}

unsigned long long getPrefetchWindowSize() {
  unsigned long long currentPrefetchWindowSize;
  unsigned long long cacheHit = hit - lastHit;
  lastHit = hit;

  if (cacheHit == 0) {
    if (curr_offset == major_trend.value_or(-1)) {
      currentPrefetchWindowSize = 1;
    } else {
      currentPrefetchWindowSize = 0;
    }
  } else {
    currentPrefetchWindowSize = std::pow(2, std::ceil(std::log2(cacheHit+1)));
  }

  currentPrefetchWindowSize = std::min(currentPrefetchWindowSize, maxPrefetchWindowSize);

  if (currentPrefetchWindowSize < lastPrefetchWindowSize/2) {
    currentPrefetchWindowSize = lastPrefetchWindowSize/2;
  }
  lastPrefetchWindowSize = currentPrefetchWindowSize;
  return currentPrefetchWindowSize;
}

void leap() {
  unsigned long long prefetchWindowSize = getPrefetchWindowSize() * PAGE_SIZE;
  if (prefetchWindowSize == 0) {
    return;
  }

  std::optional<off64_t> delta_maj = findTrend();
  if (delta_maj.has_value()) {
    // read PAGE_SIZE blocks using stride delta_maj
    for (unsigned long long i = 0; i < prefetchWindowSize; i++) {
      cache->insert(curr_offset + delta_maj.value() * i);
    }
  } else {
    // read PAGE_SIZE blocks using stride major_trend
    if (major_trend.has_value()) {
      for (unsigned long long i = 0; i < prefetchWindowSize; i++) {
        cache->insert(curr_offset + major_trend.value() * i);
      }
    }
  }
}
