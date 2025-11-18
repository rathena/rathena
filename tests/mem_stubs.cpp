#include <cstdlib>
extern "C" {
void* _mmalloc(unsigned long size, const char* file, int line, const char* func) { return malloc(size); }
void* _mcalloc(unsigned long n, unsigned long size, const char* file, int line, const char* func) { return calloc(n, size); }
void* _mrealloc(void* ptr, unsigned long size, const char* file, int line, const char* func) { return realloc(ptr, size); }
void  _mfree(void* ptr, const char* file, int line, const char* func) { free(ptr); }
}