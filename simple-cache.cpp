#include <unordered_map>
#include <list>

template <typename K, typename V>
class Cache {
private:
  int capacity = 0; // The maximum number of elements in the cache
  std::list<std::pair<K, V>> cacheList; // The list of elements in the cache
  std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> cacheMap; // The map of elements in the cache

public:
  Cache(int capacity) : capacity(capacity) {}

  V get(K key) {
    if (cacheMap.fin(key) != cacheMap.end()){
      auto it = cacheMap[key];
      cacheList.push_front(*it);
      cacheMap[key] = cacheList.begin();
      auto val = it->second;
      cacheList.erase(it);
      return val;
    }
    return nullptr; // MISS
  }

  void put(K key, V value) {
    if (cacheMap.find(key) != cacheMap.end()) {
      auto it = cacheMap[key];
      cacheList.push_front(std::make_pair(key, value));
      cacheMap[key] = cacheList.begin();
      cacheList.erase(it);
    } else {
      if (cacheMap.size() == capacity) {
        cacheMap.erase(cacheList.back().first);
        cacheList.pop_back();
      }
      cacheList.push_front(std::make_pair(key, value));
      cacheMap[key] = cacheList.begin();
    }
  }
};

int main() {
  return 0;
}

/* Some notes:
 * 1. Class is easier to code and maintain than a struct
 * 2. Even though struct is faster, it's negligible (virtual function is the devil)
 * 3. The code is not thread-safe (yet)
 * 4. The use of unordered_map and list is to achieve O(1) time complexity for both insert and delete
 * 5. The use of iterator is to achieve O(1) time complexity for both insert and delete
 * 6. Still finding the best way to access HDD (or SSD) for the cache data
 * 7. The code is semi LRU (Least Recently Used) cache for the list storage
 * */
