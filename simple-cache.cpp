#include <unordered_map>
#include <list>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// implement prefetching algorithm --> sequential prefetching || next-line prefetcher --> make it modular as well
// offset 1 and offset 2 --> miss --> prefetch 3

// implement another prefetching algorithm from the paper below (leap)
// https://www.usenix.org/system/files/atc20-maruf.pdf
// the idea is looking at the different in offset of consecutive I/Os
// if delta is small then prefetch the same size of delta
// careful about the latency caused by the heuristic

// use latency + hit rate with prefetch and without prefetch to compare the performance
// LRU + sequential prefetching
// LFU + sequential prefetching
// LRU + leap prefetching
// LFU + leap prefetching

// NOTE: AVOID HARD CODING VARIABLES

// variables from the command line
const char *mountingPoint;
long capacity;
std::ifstream traceFile;


// global variables for the cache
int hit = 0;
int total = 0;
int fd; // file descriptor for the mounting point

// function prototypes for the eviction and prefetching algorithms
void LRU();
void LFU();
void sequentialPrefetching();
void leapPrefetching();


// function pointers for the eviction and prefetching algorithms
void (*evict)();
void (*prefetch)();

// function to return the eviction algorithm based on the command line argument
void (*evictionAlgorithm(const std::string& algorithm))() {
    if (algorithm == "LRU") {
        return LRU;
    } else if (algorithm == "LFU") {
        return LFU;
    } else {
        throw std::invalid_argument("Invalid eviction algorithm");
    }
}

// function to return the prefetch algorithm based on the command line argument
void (*prefetchAlgorithm(const std::string& algorithm))() {
    if (algorithm == "Sequential") {
        return sequentialPrefetching;
    } else if (algorithm == "Leap") {
        return leapPrefetching;
    } else {
        throw std::invalid_argument("Invalid prefetch algorithm");
    }
}

int main(int argc, char *argv[]) {
  // TODO: add command line arguments for algorithm, trace file, mounting point, and cache capacity
  if (argc < 6) {
    std::cerr << "Usage: " << argv[0] << " <mounting point> <trace file> <eviction algorithm> <prefetch algorithm> <cache capacity>" << std::endl;
    return 1;
  }
  
  mountingPoint = argv[1];
  traceFile.open(argv[2]);
  evict = evictionAlgorithm(argv[3]);
  prefetch = prefetchAlgorithm(argv[4]);
  capacity = std::stol(argv[5]);

  if (traceFile.fail()) {
    std::cerr << "Error opening trace file" << std::endl;
    return 1;
  }

  // access the mounting point
  fd = open(mountingPoint, O_RDONLY);
  if (fd == -1) {
    perror("Error opening file");
    throw std::runtime_error("Error opening file");
  }

  // run the eviction algorithm
  evict();

  // close the access to the mounting point
  close(fd);

  // calculating the hit ratio
  double hitRatio = double(hit) / double(total);
  std::cout << "Hit Ratio: " << hitRatio << std::endl;

  return 0;
}

void LRU() {
  // cache structure to store the frequency of each byte
  char temp[1]; // temporary buffer for reading bytes from the disk
  std::list<char> cacheList; // the list of cached bytes
  std::unordered_map<off64_t, typename std::list<char>::iterator> cacheMap; // point offset to the location in the list


  // read the trace file line by line
  std::string line;
  while (std::getline(traceFile, line)) {
    std::istringstream iss(line);
    double timestamp;
    int deviceId, size, rw;
    off64_t offset;

    // dissect the line into the variables
    if (!(iss >> timestamp >> deviceId >> offset >> size >> rw)) {
      throw std::runtime_error("Error parsing trace file");
    }

    // only consider read operations (rw == 1)
    if (rw != 1) {
      continue;
    }

    for(int i = 0; i < size; i++){
      // if the byte is not in the cache, read it from the disk and add it to the cache
      if (cacheMap.find(offset+i) == cacheMap.end()) {
        // if the cache is full, remove the last element
        if (cacheMap.size() == capacity) {
          auto last = cacheList.back();
          cacheList.pop_back();
          cacheMap.erase(last);
        }

        // read the byte from the disk
        ssize_t bytesRead = pread(fd, temp, 1, offset+i);
        if (bytesRead == -1) {
          perror("Error reading file");
          throw std::runtime_error("Error reading file");
        }

        // add the byte to the cache (at the front of the list for the LRU policy)
        cacheList.emplace_front(temp[0]);
        cacheMap[offset+i] = cacheList.begin();
      } else {
        // if the byte is in the cache, move it to the front of the list
        cacheList.splice(cacheList.begin(), cacheList, cacheMap[offset+i]);
        cacheMap[offset+i] = cacheList.begin();
        hit++;
      }
      total++;
    }
  }
}

void LFU() {
  // cache structure to store the frequency of each byte
  struct {
    int frequency;
    char data;
  } typedef Cache;

  // cache structure to store the frequency of each byte
  char temp[1]; // temporary buffer for reading bytes from the disk
  std::list<Cache> cacheList; // the list of cached bytes
  std::unordered_map<off64_t, typename std::list<Cache>::iterator> cacheMap; // point an offset to the corresponding byte in the cache


  // read the trace file line by line
  std::string line;
  while (std::getline(traceFile, line)) {
    std::istringstream iss(line);
    double timestamp;
    int deviceId, size, rw;
    off64_t offset;

    // dissect the line into the variables
    if (!(iss >> timestamp >> deviceId >> offset >> size >> rw)) {
      throw std::runtime_error("Error parsing trace file");
    }

    // only consider read operations (rw == 1)
    if (rw != 1) {
      continue;
    }

    for(int i = 0; i < size; i++){
      // if the byte is not in the cache, read it from the disk and add it to the cache
      if (cacheMap.find(offset+i) == cacheMap.end()) {
        // if the cache is full, remove the least frequently used elemen
        if (cacheMap.size() == capacity) {
          auto last = std::min_element(cacheList.begin(), cacheList.end(), [](const Cache& a, const Cache& b) {
            return a.frequency < b.frequency;
          });
          cacheMap.erase(last->frequency);
          cacheList.erase(last);
        }

        // read the byte from the disk
        ssize_t bytesRead = pread(fd, temp, 1, offset+i);
        if (bytesRead == -1) {
          perror("Error reading file");
          throw std::runtime_error("Error reading file");
        }

        // add the cached data to the cacheList
        cacheList.emplace_front(Cache{1, temp[0]});
        cacheMap[offset+i] = cacheList.begin();
      } else {
        // if the byte is in the cache, add its frequency
        cacheMap[offset+i]->frequency++;
        hit++;
      }
      total++;
    }
  }
}

void sequentialPrefetching() {
  std::cout << "Sequential Prefetching" << std::endl;
}

void leapPrefetching() {
  std::cout << "Leap Prefetching" << std::endl;
}