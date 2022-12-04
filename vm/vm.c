/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)
	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	printf("check vm_alloc vm_type : %d\n", type);

	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initializer according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		struct page *new_pg = (struct page *)malloc(sizeof(struct page));
		if (new_pg == NULL) goto err;
		printf("check before switch %d\n", type);
		switch (VM_TYPE(type))
		{
			case VM_ANON:
				printf("check vm_anon");
				uninit_new (new_pg, upage, init, type, aux, anon_initializer);
				/* code */
				break;
			case VM_FILE:
				uninit_new (new_pg, upage, init, type, aux, file_backed_initializer);
				break;
			case VM_PAGE_CACHE:
				break;
			default:
				break;
		}
		/* TODO: Insert the page into the spt. */
		spt_insert_page(spt, new_pg);
	}
	return true;
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
// spt에서 해당 va를 가지고 있는 elem을 찾아서 그에 맞는 page를 반환하는 함수
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = (struct page *)malloc(sizeof page);	// malloc을 통해서 빈 page를 만들어준다.
	if (page == NULL) return NULL;
	struct page *ret = NULL;
	struct hash_elem *e = NULL;
	page->va = pg_round_down(va); // va가 가르키는 페이지의 시작인 va (해당 페이지의 offset 0 주소)를 리턴해줌.
	e = hash_find(&spt->spt_hash, &page->hash_elem);	// spt 내에서 해당 페이지를 찾는다.
	if (e)
		ret = hash_entry(e, struct page, hash_elem);
	else {
		free(page);
		return NULL;
	}
	free(page);
	/* TODO: Fill this function. */
	return ret;
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function.c */
	if (!hash_insert(&spt->spt_hash, &page->hash_elem))
		succ = true;
	return succ;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = NULL;
	/* TODO: Fill this function. */
	frame = (struct frame *)malloc(sizeof frame);
	frame->kva = palloc_get_page(PAL_USER);
	frame->page = NULL;
	/*
	TODO : if user pool memory is full, do evict.
	*/
	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	if (is_kernel_vaddr(addr)){
		return false;
	}
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */
	printf("=============check vm_try_handle_fault ==============\n");
	printf("rsp=%p\nfault_addr=%p\n user=%d\n write=%d\n not_present=%d\n", f->rsp, addr, user, write, not_present);
	if (page = spt_find_page(spt, addr) == NULL || true)
	{
	
		printf("=============check vm_try_handle_fault_page_fault ture ==============\n");
		return vm_do_claim_page (page);
	}
	else
		return false;
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */
	page = spt_find_page (&thread_current()->spt, va);
	printf("in vm_claim_page page = %p\n", page);
	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();
	/* Set links */
	printf("===========do_cl===============\n");
	frame->page = page;
	page->frame = frame;
	printf("page->frame->kva %p\n", page->frame->kva);
	printf("page->operations%p\n",page->operations);
	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	if (pml4_get_page(thread_current()->pml4, page->va) == NULL) // table에 해당하는 값이 없으면
		if (!pml4_set_page(thread_current()->pml4, page->va, frame->kva, 1)) // table에 해당하는 page 넣어주고,
			{
				return false;
			}
	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	hash_init(&(spt->spt_hash), page_hash, page_less, 0);
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}

unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED) {
  const struct page *p = hash_entry (p_, struct page, hash_elem);
  return hash_bytes (&p->va, sizeof p->va);
//   return hash_bytes (&p->addr, sizeof p->addr);
}

bool
page_less (const struct hash_elem *a_,
           const struct hash_elem *b_, void *aux UNUSED) {
  const struct page *a = hash_entry (a_, struct page, hash_elem);
  const struct page *b = hash_entry (b_, struct page, hash_elem);

//   return a->addr < b->addr;
return a->va < b->va;
}

struct page *
page_lookup (const void *address) {
  struct page p;
  struct hash_elem *e;

  p.va = address;
  e = hash_find (&thread_current()->spt.spt_hash, &p.hash_elem);
  return e != NULL ? hash_entry (e, struct page, hash_elem) : NULL;
}