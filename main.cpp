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
#include <sys/stat.h>

// variables from the command line
const char *mountingPoint;
long capacity;
std::ifstream traceFile;

// TODO: move to a header files for cleaner code
// global variables for the cache
int hit = 0;
int total = 0;
int fd; // file descriptor for the mounting point
char temp[10000]; // temporary buffer for reading bytes from the disk
ssize_t BLOCKSIZE; // block size of the disk

// function prototypes for the eviction and prefetching algorithms
void LRU();
void LFU();

ssize_t noPrefetching(off64_t);
ssize_t sequentialPrefetching(off64_t);
ssize_t leapPrefetching(off64_t);

// function pointers for the eviction and prefetching algorithms
void (*evict)();
ssize_t (*prefetch)(off64_t); //TODO: change to args or smth like argv in main

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
ssize_t (*prefetchAlgorithm(const std::string& algorithm))(off64_t) {
    if (algorithm == "Sequential") {
        return sequentialPrefetching;
    } else if (algorithm == "Leap") {
        return leapPrefetching;
    } else if (algorithm == "None") {
      return noPrefetching;
    } else {
        throw std::invalid_argument("Invalid prefetch algorithm");
    }
}

int main(int argc, char *argv[]) {
  if (argc < 6) {
    std::cerr << "Usage: " << argv[0] << " <mounting point> <trace file> <eviction algorithm> <prefetch algorithm> <cache capacity>" << std::endl;
    return 1;
  }

  // setting the command line arguments
  mountingPoint = argv[1];
  traceFile.open(argv[2]);
  evict = evictionAlgorithm(argv[3]);
  prefetch = prefetchAlgorithm(argv[4]);
  capacity = std::stol(argv[5]);
  if (traceFile.fail()) {
    std::cerr << "Error opening trace file" << std::endl;
    return 1;
  }

   // setting the block size of the disk
  struct stat st{};
  if (stat(mountingPoint, &st) == -1) {
    perror("Error getting file status");
    throw std::runtime_error("Error getting file status");
  }
  BLOCKSIZE = st.st_blksize;

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
        // read the byte from the disk
        ssize_t bytesRead = prefetch(offset+i);

        for(int j = 0; i < bytesRead; i++) {
          if (cacheMap.size() == capacity) {
            auto last = cacheList.back();
            cacheList.pop_back();
            cacheMap.erase(last);
          }
          cacheList.emplace_front(temp[j]);
          cacheMap[offset+i] = cacheList.begin();
        }

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

        // read the byte from the disk
        ssize_t bytesRead = prefetch(offset+i);
        /*
         * pread(size, offset) ...
         * char temp[]....
         * prefetch()??
         *      pread... but go to cache map directly
         *
         *
         *
         * [???]
         * the pread for miss and prefetch should be different since prefetch can also be done when hit (not only miss)
         * */

        for (int j = 0; j < bytesRead; j++) {
          // if the cache is full, remove the least frequently used element
          if (cacheMap.size() == capacity) {
            auto last = std::min_element(cacheList.begin(), cacheList.end(), [](const Cache& a, const Cache& b) {
              return a.frequency < b.frequency;
            });
            cacheMap.erase(last->frequency);
            cacheList.erase(last);
          }

          // add the cached data to the cacheList
          cacheList.emplace_front(Cache{1, temp[j]});
          cacheMap[offset+i] = cacheList.begin();
        }

      } else {
        // if the byte is in the cache, add its frequency
        cacheMap[offset+i]->frequency++;
        hit++;
      }
      total++;
    }
  }
}

ssize_t noPrefetching(off64_t offset) {
  ssize_t bytesRead = pread(fd, temp, 1, offset);
  if (bytesRead == -1) {
    perror("Error reading file");
    throw std::runtime_error("Error reading file");
  }
  return bytesRead;
}

// https://www.ibm.com/docs/en/db2/11.5?topic=pool-sequential-prefetching
// the idea is read data contiguously from the disk (sized of BLOCKSIZE currently)
ssize_t sequentialPrefetching(off64_t offset) {
  ssize_t bytesRead = pread(fd, temp, BLOCKSIZE, offset);
  if (bytesRead == -1) {
    perror("Error reading file");
    throw std::runtime_error("Error reading file");
  }
  return bytesRead;
}

// https://www.usenix.org/system/files/atc20-maruf.pdf
// the idea is looking at the different in offset of consecutive I/Os
// if delta is small then prefetch the same size of delta
// careful about the latency caused by the heuristic
ssize_t leapPrefetching(off64_t offset) {
  std::cout << "Leap Prefetching" << std::endl;
  return -1;
}