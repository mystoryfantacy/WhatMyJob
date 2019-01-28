#include <iostream>
#include <unordered_map>
#include <vector>

class SimpleMemoryPool {
private:
  struct Node {
    Node(void *p_) {
      p = p_;
      next = nullptr;
    }
    void *p;
    Node *next;
    ~Node() {
      if (p != nullptr)
        free(p);
      if (next != nullptr)
        delete next;
    }
  };

public:
  ~SimpleMemoryPool() {
    for (size_t i = 0; i < used_block_.size(); ++i) {
      delete used_block_[i];
      delete free_block_[i];
    }
  }

  void *Alloc(size_t size) {
    void *p = nullptr;
    size_t block_size = GetBlockSize(size);

    size_t next_block_size = block_size;
    int id = -1;
    printf("----------------------------\n");
    for (size_t i = 0; i < kSTEP; ++i) {
      printf("block_size %d\n", next_block_size);
      id = TryGetFreeBlockIdBySize(next_block_size);
      if (id != -1)
        break;
      next_block_size = GetNextBlockSize(next_block_size);
    }
    if (id == -1) {
      p = malloc(block_size);
      id = GetIdBySize(block_size);
      Node *node = new Node(nullptr);
      node->next = used_block_[id]->next;
      used_block_[id]->next = node;
    } else {
      Node *tmp = free_block_[id]->next;
      free_block_[id]->next = tmp->next;
      tmp->next = used_block_[id]->next;
      used_block_[id]->next = tmp;
      p = tmp->p;
      tmp->p = nullptr;
    }
    pointer2id_[p] = id;
    return p;
  }

  void Free(void *p) {
    size_t id = pointer2id_[p];
    Node *tmp = used_block_[id]->next;
    used_block_[id]->next = tmp->next;
    tmp->p = p;
    tmp->next = free_block_[id]->next;
    free_block_[id]->next = tmp;
  }

  void MemoryInfo() {
    std::cout << "Total Entries: " << free_block_.size() << std::endl;
    std::cout << ">> Size Id View:\n";
    for(auto it = size2id_.begin(); it != size2id_.end(); ++it) {
      std::cout << "  " << it->first << " " << it->second << std::endl;
    }
  }

private:
  size_t GetBlockSize(size_t real_size) {
    size_t block_size = 1;
    while(real_size > 0) {
      real_size >>= 1;
      block_size <<= 1;
    }
    return block_size;
  }

  int TryGetFreeBlockIdBySize(size_t block_size) {
    if (size2id_.find(block_size) != size2id_.end()) {
      int id = size2id_[block_size];
      if (free_block_[id]->next != nullptr)
        return id;
    }
    return -1;
  }

  int GetIdBySize(size_t block_size) {
    if (size2id_.find(block_size) != size2id_.end()) {
      return size2id_[block_size];
    } else {
      used_block_.push_back(new Node(nullptr));
      free_block_.push_back(new Node(nullptr));
      int id = UID++;
      size2id_[block_size] = id;
      return id;
    }
  }

  size_t GetNextBlockSize(size_t block_size) {
    if (block_size < 1024)
      return (block_size << 1);
    else {
      return ((block_size + 1023) >> 10) << 10;
    }
  }

  std::vector<Node *> used_block_;
  std::vector<Node *> free_block_;
  int UID = 0;
  const size_t kSTEP = 4;
  std::unordered_map<size_t, int> size2id_;
  std::unordered_map<void *, int> pointer2id_;
};

static SimpleMemoryPool __mem;

#define ALLOC(size) __mem.Alloc(size)
#define FREE(p) __mem.Free(p)
#define MEM_INFO __mem.MemoryInfo()

// Try to implement it with all pointers instead of vectors
/*
class SimpleMemoryPool {
  std::unordered_map<void*, size_t> addr2size_;
  std::unordered_map<size_t, void*> free_block;
  struct BlockInfo {
    char status;
    size_t size;
    void* block;
    BlockInfo* next;
  }

  size_t GetBlockSize(size_t real_size) {
    size_t block_size = (real_size & (real_size-1)) << 1;
    return block_size;
  }

  void* Alloc(size_t size) {
    void* p = nullptr;
    size_t block_size = GetBlockSize(size);
    if (free_block.find(block_size) != free_block.end()) {
      BlockInfo* cursor = free_block[block_size];
      for(int i = 0; i < 4; i++) {
        if (cursor->next->status == 'H') {
          cursor = cursor->next;
        }
        if (cursor->next == nullptr)
          break;
        p = cursor->next->block;
        BockInfo* tmp = cursor->next;
        cursor->next = tmp->next;
        BlockInfo* used = used_block[block_size];
        tmp->next = used->next;
        used->next = tmp;
      }
    }
  }
}
*/
