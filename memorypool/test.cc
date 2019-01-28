#include "simple_memory_pool.h"

int main() {
 std::vector<void*> ps;
 int max_num = 1000;
 for(int i = 1; i < max_num; i++) {
   ps.push_back(ALLOC(i * 100));
 }
 MEM_INFO;
 for(int i = 1; i < max_num; i++) {
   FREE(ps[i]);
 }
 return 0;
}
