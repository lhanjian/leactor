#include "event_lea.h"
#include "lt_pool.h"

lt_memory_pool_manager_t *
lt_new_memory_pool_manager(lt_memory_pool_manager_t *manager, size_t
		one_element_size, size_t element_count)
{
	if (!manager) {
		manager = calloc(1, sizeof(lt_memory_pool_manager_t));
	}
	manager->one_element_size = one_element_size;
	manager->element_count = element_count;
	manager->head = manager->tail = manager->cur = lt_new_memory_pool(manager);

	return manager;
}

void *
lt_alloc(lt_memory_pool_manager_t *manager)
{
	uintptr_t new_space;
	lt_memory_pool_t *cur = manager->tail;
	size_t element_size = manager->one_element_size;

	if (cur->pos > cur->end) {
		lt_memory_pool_t *new_pool = lt_new_memory_pool(manager);
		cur->next = new_pool;
		cur = new_pool;
	}

	new_space = cur->pos;
	cur->pos = alignment_ptr(cur->pos, sizeof(uintptr_t)) + element_size;

	return new_space;
}

lt_memory_pool_t *
lt_new_memory_pool(lt_memory_pool_manager_t *manager)
{
	size_t element_size = manager->one_element_size;
	size_t size_of_pool_struct_and_all_elements =
			sizeof(lt_memory_pool_manager_t)
			+ manager->element_count * element_size;

	lt_memory_pool_t *mem = aligned_alloc(ALIGNMENT, size_of_pool_struct_and_all_elements);
	if (mem == NULL) {
		perror("aligned_alloc: ");
		return NULL;
	}

	mem->start = (uintptr_t) mem + sizeof(lt_memory_pool_t);
	mem->pos = alignment_ptr(mem->start, sizeof(uintptr_t)) + element_size;
	mem->end = (uintptr_t) mem + size_of_pool_struct_and_all_elements;
	mem->next = NULL;
	mem->manager = manager;

	if (manager) {
		manager->cur = mem;
		manager->tail->next = mem;
		manager->tail = mem;
	}

	return mem;
}

void
lt_destroy_memory_pool(lt_memory_pool_manager_t *manager)
{
	lt_memory_pool_t *pool = manager->head;
	lt_memory_pool_t *next;
	for (;;) {
		next = pool->next;
		free(pool);
		if (next == NULL) {
			break;
		}
	}

}
