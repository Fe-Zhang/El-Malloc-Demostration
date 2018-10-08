#ifndef EL_MALLOC_H
#define EL_MALLOC_H 1

// macro to add a byte offset to a pointer, arguments are a pointer
// and a # of bytes (usually size_t)
#define PTR_PLUS_BYTES(ptr,off) ((void *) (((size_t) (ptr)) + ((size_t) (off))))

// macro to add a byte offset to a pointer, arguments are a pointer
// and a # of bytes (usually size_t)
#define PTR_MINUS_BYTES(ptr,off) ((void *) (((size_t) (ptr)) - ((size_t) (off))))

// macro to add a byte offset to a pointer, arguments are a pointer
// and a # of bytes (usually size_t)
#define PTR_MINUS_PTR(ptr,ptq) (((size_t) (ptr)) - ((size_t) (ptq)))

// defines to indicate if a block is available or used
#define EL_AVAILABLE     'a'    // block state indicating available
#define EL_USED          'u'    // block state indicating in use
#define EL_BEGIN_BLOCK   'B'    // block state indicating dummy beginning node in a list
#define EL_END_BLOCK     'E'    // block state indicating dummy ending node in a list
#define EL_UNINITIALIZED -7     // indication of uninitialized data

// type which is a "header" for a block of memory; containts info on
// size, whether the block is available or in use, and links to the
// next/prev blocks in a doubly linked list. This data structure
// appears immediately before a block of memory that is tracked by the
// allocator.
typedef struct block {
  size_t size;                  // number of bytes of memory in this block
  char state;                   // either EL_AVAILABLE or EL_USED
  struct block *next;           // pointer to next block in same list
  struct block *prev;           // pointer to previous block in same list
} el_blockhead_t;

// Type for the "footer" of a block; indicates size of the preceding
// block so that its header el_blockhead_t can be found with pointer
// arithmetic. This data appears immediately after an area of memory
// that may be used by a user or is free. Immediately after it is
// either another header (el_blockhead_t) or the end of the heap.
typedef struct {
  size_t size;
} el_blockfoot_t;

// Size of tracking data for each block of data allocated which is a
// combination of the size of the header and footer.
#define EL_BLOCK_OVERHEAD (sizeof(el_blockhead_t) + sizeof(el_blockfoot_t))

// Type for a list of blocks; doubly linked with a fixed
// "dummy" node at the beginning and end which do not contain any
// data. List tracks its length and number of bytes in use.
typedef struct {
  el_blockhead_t beg_actual;    // fixed node at beginning of list; state is EL_BEGIN_BLOCK
  el_blockhead_t end_actual;    // fixed node at end of list; state is EL_END_BLOCK
  el_blockhead_t *beg;          // pointer to beg_actual
  el_blockhead_t *end;          // pointer to end_actual
  size_t length;                // length of the used block list (not counting beg/end)
  size_t bytes;                 // total bytes in list used including overhead; 
} el_blocklist_t;
// NOTE: total available bytes for use/in-use in the list is (bytes - length*EL_BLOCK_OVERHEAD)

// Type for the global control of the allocator. Tracks heap size,
// start and end addresses, total size, and lists of available and
// used blocks.
typedef struct {
  void *heap_start;             // pointer to where the heap starts
  void *heap_end;               // pointer to where the heap ends; this memory address is out of bounds
  size_t heap_bytes;            // number of bytes currently in the heap
  el_blocklist_t avail_actual;  // space for the available list data
  el_blocklist_t used_actual;   // space for the used list data
  el_blocklist_t *avail;        // pointer to avail_actual
  el_blocklist_t *used;         // pointer to used_actual
} el_ctl_t;

// global control declared in el_malloc.c
extern el_ctl_t el_ctl;

// functions in el_malloc.c
int  el_init(int max_bytes);
void el_print_stats();
void el_cleanup();

el_blockfoot_t *el_get_footer(el_blockhead_t *block);
el_blockhead_t *el_get_header(el_blockfoot_t *foot);
el_blockhead_t *el_block_above(el_blockhead_t *block);
el_blockhead_t *el_block_below(el_blockhead_t *block);

void el_init_blocklist(el_blocklist_t *list);
void el_print_blocklist(el_blocklist_t *list);
void el_add_block_front(el_blocklist_t *list, el_blockhead_t *block);
void el_remove_block(el_blocklist_t *list, el_blockhead_t *block);

el_blockhead_t *el_find_first_avail(size_t size);
el_blockhead_t *el_split_block(el_blockhead_t *block, size_t new_size);
el_blockhead_t *el_allocate_block(size_t size);
void *el_malloc(size_t nbytes);

void el_merge_block_with_above(el_blockhead_t *lower);
void el_free(void *ptr);

#endif
