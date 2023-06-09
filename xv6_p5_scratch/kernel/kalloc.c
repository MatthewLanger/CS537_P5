// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "spinlock.h"

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;

  /*
  P5 changes
  */
  uint free_pages; //track free pages
  uint ref_cnt[PHYSTOP / PGSIZE]; //track reference count

} kmem;

// ADDED FUNCTION: increments ref_cnt based on addr
void
increment(char *v) {
  acquire(&kmem.lock);
    int addr = (uint)v;
    int index = addr / PGSIZE;
    kmem.ref_cnt[index]++; 
  release(&kmem.lock);
}

// ADDED FUNCTION: decrements ref_cnt based on addr
void
decrement(char *v) {
  acquire(&kmem.lock);
   int addr = (uint)v;
   int index = addr / PGSIZE;
   kmem.ref_cnt[index]--; 
  release(&kmem.lock);

}

extern char end[]; // first address after kernel loaded from ELF file

// Initialize free list of physical pages.
void
kinit(void)
{
  char *p;
  initlock(&kmem.lock, "kmem");
  p = (char*)PGROUNDUP((uint)end);
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || (uint)v >= PHYSTOP) 
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);
  acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
  decrement(v); // decreases ref_cnt when page freed, might need to be set to zero
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);
  increment((char*)r); // increments ref_cnt when page added
  return (char*)r;
}

int
countFree(void) {
  int count = 0;
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  while (r->next != NULL) {
    count++;
    r = r->next; 
  }
  kmem.free_pages = count;
  
  release(&kmem.lock);
  return count;
}

