#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SIZE_T_SIZE ALIGN(sizeof(size_t))
#define BIT ((size_t)1)
#define BLOCK_SIZE(h) (*(h) & ~BIT)
#define IS_ALLOC(h) ((*(h) & BIT) != 0)
#define SET_ALLOC(h) (*(h) |= BIT)
#define CLR_ALLOC(h) (*(h) &= ~BIT)
#define H_FROM_F(f) ((size_t *)((char *)(f) + SIZE_T_SIZE) - BLOCK_SIZE(f))
#define F_FROM_H(h) ((size_t *)((char *)(h) + BLOCK_SIZE(h) - SIZE_T_SIZE))

char *g_heap_start = NULL;

void *heap_start(void) {
  if (!g_heap_start) {
    g_heap_start = (char *)sbrk(0);
  }
  return (void *)g_heap_start;
}

void *heap_end(void) { return sbrk(0); }

void *find_fit(size_t needed) {
  char *start = (char *)heap_start();
  char *end = (char *)heap_end();

  while (start < end) {
    size_t *header = (size_t *)start;
    size_t sz = BLOCK_SIZE(header);
    if (!IS_ALLOC(header) && sz >= needed) {
      return header;
    }
    start += sz;
  }
  return NULL;
}

void *_malloc(size_t size) {
  if (size == 0)
    return NULL;
  size_t block_size = ALIGN(size + 2 * SIZE_T_SIZE);
  size_t *header = (size_t *)find_fit(block_size);
  if (header) {
    SET_ALLOC(header);
    size_t *footer = F_FROM_H(header);
    *footer = *header;
    return (char *)header + SIZE_T_SIZE;
  } else {
    header = (size_t *)sbrk(block_size);
    if (header == (void *)-1) {
      return NULL;
    }
    *header = block_size | BIT;
    size_t *footer = F_FROM_H(header);
    *footer = *header;
    return (char *)header + SIZE_T_SIZE;
  }
  return NULL;
}

void _free(void *ptr) {
  if (ptr == NULL) {
    return;
  }

  size_t *header = (size_t *)((char *)ptr - SIZE_T_SIZE);
  CLR_ALLOC(header);

  char *next = (char *)header + BLOCK_SIZE(header);
  if (next + SIZE_T_SIZE <= (char *)heap_end() && !IS_ALLOC((size_t *)next) &&
      BLOCK_SIZE(header) > 0 && BLOCK_SIZE((size_t *)next) > 0) {
    *header = BLOCK_SIZE(header) + BLOCK_SIZE((size_t *)next);
  }

  char *prev_footer = (char *)header - SIZE_T_SIZE;
  if (prev_footer >= (char *)heap_start() && !IS_ALLOC((size_t *)prev_footer)) {
    size_t *prev_header = (size_t *)H_FROM_F((size_t *)prev_footer);
    if ((char *)prev_header >= (char *)heap_start()) {
      *prev_header += BLOCK_SIZE(header);
      header = prev_header;
    }
  }

  size_t *footer = F_FROM_H(header);
  *footer = *header;
}

int main(void) {
  void *a = _malloc(10);
  void *b = _malloc(24);
  printf("a = %p\nb = %p\n", a, b);

  _free(a);
  _free(b);
  void *c = _malloc(8);
  printf("c = %p (reused a? %s)\n", c, (c == a ? "yes" : "no"));

  size_t heap_size = (char *)heap_end() - (char *)heap_start();
  printf("Heap size: %zu bytes (%.2f KB, %.2f MB)\n", heap_size,
         heap_size / 1024.0, heap_size / (1024.0 * 1024.0));

  return 0;
}
