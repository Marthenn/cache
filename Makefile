clean:
	rm -f *.o *.a *.so *.out *.exe *.dll *.dylib *.app *.dSYM

cache:
	g++ -fPIC -o cache.out main.cpp variables.cpp eviction/eviction.cpp eviction/lru.cpp eviction/lfu.cpp prefetch/prefetch.cpp prefetch/sequential.cpp prefetch/leap.cpp