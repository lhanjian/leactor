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
	manager->cur = NULL;
	manager->cur = lt_new_memory_pool(manager);
	manager->tail = manager->cur;
	manager->head = manager->cur;

	return manager;
}

void *
lt_alloc(lt_memory_pool_manager_t *manager)
{
	uintptr_t new_space;
	lt_memory_pool_t *cur = manager->tail;
	size_t element_size = manager->one_element_size;

	new_space = cur->pos;
	cur->pos = alignment_ptr(cur->pos + element_size, sizeof(uintptr_t));

	if (cur->pos > cur->end) {
		/*lt_memory_pool_t *new_pool = */
		lt_new_memory_pool(manager);
		//cur->next = new_pool;
		//cur = new_pool;
	}

	return (void *)new_space;
}

lt_memory_pool_t *
lt_new_memory_pool(lt_memory_pool_manager_t *manager)
{
	size_t element_size = manager->one_element_size;
	size_t size_of_pool_struct_and_all_elements =
			sizeof(lt_memory_pool_manager_t)
			+ manager->element_count * element_size;

	lt_memory_pool_t *pool = aligned_alloc(ALIGNMENT, size_of_pool_struct_and_all_elements);
	if (pool == NULL) {
		perror("aligned_alloc: ");
		return NULL;
	}
// mem { lt_memory_pool_t + user_data + user_data + .... + user_data  }
	pool->start = (uintptr_t) pool /*+ sizeof(lt_memory_pool_t)*/;
	pool->pos = alignment_ptr(
			pool->start + sizeof(lt_memory_pool_t),
			sizeof(uintptr_t))/*+ element_size*/;
	pool->end = (uintptr_t) pool + size_of_pool_struct_and_all_elements;
	pool->next = NULL;
	pool->manager = manager;



	if (manager->cur) {
		manager->tail->next = pool;
	}
	manager->cur = pool;
	manager->tail = pool;

	return pool;
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
