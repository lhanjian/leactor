#ifndef _INCLUDE_LEA_POOL_H
#define _INCLUDE_LEA_POOL_H
#define ALIGNMENT (16)
#define align_num(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define alignment_ptr(p, a) \
	(uintptr_t)(((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

lt_memory_pool_manager_t *
lt_new_memory_pool_manager(lt_memory_pool_manager_t *manager,
		size_t one_element_size, size_t element_count);

lt_memory_pool_t *lt_new_memory_pool(lt_memory_pool_manager_t *manager);

void
lt_destroy_memory_pool(lt_memory_pool_manager_t *manager);

void *
lt_alloc(lt_memory_pool_manager_t *manager);
#endif
