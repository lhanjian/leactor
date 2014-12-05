#define ALIGNMENT (16)
#define align_num(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define alignment_ptr(p, a) \
	(uintptr_t)(((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

typedef struct lt_memory_pool {
	uintptr_t start, end;
	uintptr_t pos;

	struct lt_memory_pool *next;
	struct lt_memory_pool_manager *manager;
} lt_memory_pool_t;

typedef struct lt_memory_pool_manager {
	lt_memory_pool_t *cur;
	size_t one_element_size;
	size_t element_count;
	struct lt_memory_pool *head;
	struct lt_memory_pool *tail;
	struct lt_memory_pool_manager *heterogeneous_next;
} lt_memory_pool_manager_t;


