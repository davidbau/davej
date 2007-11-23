/* $Id: pgtable.h,v 1.28 1997/04/14 17:05:19 jj Exp $
 * pgtable.h: SpitFire page table operations.
 *
 * Copyright 1996 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef _SPARC64_PGTABLE_H
#define _SPARC64_PGTABLE_H

/* This file contains the functions and defines necessary to modify and use
 * the SpitFire page tables.
 */

#ifndef __ASSEMBLY__
#include <linux/mm.h>
#endif
#include <asm/spitfire.h>
#include <asm/asi.h>
#include <asm/mmu_context.h>
#include <asm/system.h>

#ifndef __ASSEMBLY__
#include <asm/sbus.h>

/* Certain architectures need to do special things when pte's
 * within a page table are directly modified.  Thus, the following
 * hook is made available.
 */
#define set_pte(pteptr, pteval) ((*(pteptr)) = (pteval))

/* PMD_SHIFT determines the size of the area a second-level page table can map */
#define PMD_SHIFT	(PAGE_SHIFT + (PAGE_SHIFT-3))
#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE-1))

/* PGDIR_SHIFT determines what a third-level page table entry can map */
#define PGDIR_SHIFT	(PAGE_SHIFT + 2*(PAGE_SHIFT-3))
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

/* Entries per page directory level. */
#define PTRS_PER_PTE	(1UL << (PAGE_SHIFT-3))
#define PTRS_PER_PMD	(1UL << (PAGE_SHIFT-3))
#define PTRS_PER_PGD	(1UL << (PAGE_SHIFT-3))

#define PTE_TABLE_SIZE	0x2000	/* 1024 entries 8 bytes each */
#define PMD_TABLE_SIZE	0x2000	/* 1024 entries 8 bytes each */
#define PGD_TABLE_SIZE	0x2000	/* 1024 entries 8 bytes each */

/* the no. of pointers that fit on a page */
#define PTRS_PER_PAGE	(1UL << (PAGE_SHIFT-3))

/* NOTE: TLB miss handlers depend heavily upon where this is. */
#define VMALLOC_START		0xFFFFFc0000000000UL
#define VMALLOC_VMADDR(x)	((unsigned long)(x))

#endif /* !(__ASSEMBLY__) */

/* SpitFire TTE bits. */
#define _PAGE_VALID	0x8000000000000000	/* Valid TTE                          */
#define _PAGE_R		0x8000000000000000	/* Used to keep ref bit up to date    */
#define _PAGE_SZ4MB	0x6000000000000000	/* 4MB Page                           */
#define _PAGE_SZ512K	0x4000000000000000	/* 512K Page                          */
#define _PAGE_SZ64K	0x2000000000000000	/* 64K Page                           */
#define _PAGE_SZ8K	0x0000000000000000	/* 8K Page                            */
#define _PAGE_NFO	0x1000000000000000	/* No Fault Only                      */
#define _PAGE_IE	0x0800000000000000	/* Invert Endianness                  */
#define _PAGE_SOFT2	0x07FC000000000000	/* Second set of software bits        */
#define _PAGE_DIAG	0x0003FE0000000000	/* Diagnostic TTE bits                */
#define _PAGE_PADDR	0x000001FFFFFFE000	/* Physical Address bits [40:13]      */
#define _PAGE_SOFT	0x0000000000001F80	/* First set of software bits         */
#define _PAGE_L		0x0000000000000040	/* Locked TTE                         */
#define _PAGE_CP	0x0000000000000020	/* Cacheable in Physical Cache        */
#define _PAGE_CV	0x0000000000000010	/* Cacheable in Virtual Cache         */
#define _PAGE_E		0x0000000000000008	/* side-Effect                        */
#define _PAGE_P		0x0000000000000004	/* Privileged Page                    */
#define _PAGE_W		0x0000000000000002	/* Writable                           */
#define _PAGE_G		0x0000000000000001	/* Global                             */

/* Here are the SpitFire software bits we use in the TTE's. */
#define _PAGE_PRESENT	0x0000000000001000	/* Present Page (ie. not swapped out) */
#define _PAGE_MODIFIED	0x0000000000000800	/* Modified Page (ie. dirty)          */
#define _PAGE_ACCESSED	0x0000000000000400	/* Accessed Page (ie. referenced)     */
#define _PAGE_READ	0x0000000000000200	/* Readable SW Bit                    */
#define _PAGE_WRITE	0x0000000000000100	/* Writable SW Bit                    */
#define _PAGE_PRIV	0x0000000000000080	/* Software privilege bit	      */

#define _PAGE_CACHE	(_PAGE_CP | _PAGE_CV)

#define __DIRTY_BITS	(_PAGE_MODIFIED | _PAGE_WRITE | _PAGE_W)
#define __ACCESS_BITS	(_PAGE_ACCESSED | _PAGE_READ | _PAGE_R)
#define __PRIV_BITS	(_PAGE_P | _PAGE_PRIV)

#define PAGE_NONE	__pgprot (_PAGE_PRESENT | _PAGE_VALID | _PAGE_CACHE | \
				  __PRIV_BITS | __ACCESS_BITS)

#define PAGE_SHARED	__pgprot (_PAGE_PRESENT | _PAGE_VALID | _PAGE_CACHE | \
				  __ACCESS_BITS | _PAGE_W | _PAGE_WRITE)

#define PAGE_COPY	__pgprot (_PAGE_PRESENT | _PAGE_VALID | _PAGE_CACHE | \
				  __ACCESS_BITS)

#define PAGE_READONLY	__pgprot (_PAGE_PRESENT | _PAGE_VALID | _PAGE_CACHE | \
				  __ACCESS_BITS)

#define PAGE_KERNEL	__pgprot (_PAGE_PRESENT | _PAGE_VALID | _PAGE_CACHE | \
				  __PRIV_BITS | __ACCESS_BITS | __DIRTY_BITS)

#define PAGE_INVALID	__pgprot (0)

#define _PFN_MASK	_PAGE_PADDR

#define _PAGE_CHG_MASK	(_PFN_MASK | _PAGE_MODIFIED | _PAGE_ACCESSED | _PAGE_PRESENT)

#define pg_iobits (_PAGE_VALID | __PRIV_BITS | __ACCESS_BITS | _PAGE_E)

#define __P000	PAGE_NONE
#define __P001	PAGE_READONLY
#define __P010	PAGE_COPY
#define __P011	PAGE_COPY
#define __P100	PAGE_READONLY
#define __P101	PAGE_READONLY
#define __P110	PAGE_COPY
#define __P111	PAGE_COPY

#define __S000	PAGE_NONE
#define __S001	PAGE_READONLY
#define __S010	PAGE_SHARED
#define __S011	PAGE_SHARED
#define __S100	PAGE_READONLY
#define __S101	PAGE_READONLY
#define __S110	PAGE_SHARED
#define __S111	PAGE_SHARED

#ifndef __ASSEMBLY__

extern pte_t __bad_page(void);
extern pmd_t *__bad_pmd(void);
extern pte_t *__bad_pte(void);

#define BAD_PMD		__bad_pmd()
#define BAD_PTE		__bad_pte()
#define BAD_PAGE	__bad_page()

/* First phsical page can be anywhere, the following is needed so that
 * va-->pa and vice versa conversions work properly without performance
 * hit for all __pa()/__va() operations.
 */
extern unsigned long phys_base;

#define ZERO_PAGE	(PAGE_OFFSET + phys_base)

/* This is for making TLB miss faster to process. */
extern unsigned long null_pmd_table;
extern unsigned long null_pte_table;

/* Allocate a block of RAM which is aligned to its size.
   This procedure can be used until the call to mem_init(). */
extern void *sparc_init_alloc(unsigned long *kbrk, unsigned long size);

/* Cache and TLB flush operations. */

/* This is a bit tricky to do most efficiently.  The I-CACHE on the
 * SpitFire will snoop stores from _other_ processors and changes done
 * by DMA, but it does _not_ snoop stores on the local processor.
 * Also, even if the I-CACHE snoops the store from someone else correctly,
 * you can still lose if the instructions are in the pipeline already.
 * A big issue is that this cache is only 16K in size, using a pseudo
 * 2-set associative scheme.  A full flush of the cache is far too much
 * for me to accept, especially since most of the time when we get to
 * running this code the icache data we want to flush is not even in
 * the cache.  Thus the following seems to be the best method.
 */
extern __inline__ void spitfire_flush_icache_page(unsigned long page)
{
	unsigned long temp;

	/* Commit all potential local stores to the instruction space
	 * on this processor before the flush.
	 */
	membar("#StoreStore");

	/* Actually perform the flush. */
	__asm__ __volatile__("
1:
	flush		%0 + 0x00
	flush		%0 + 0x08
	flush		%0 + 0x10
	flush		%0 + 0x18
	flush		%0 + 0x20
	flush		%0 + 0x28
	flush		%0 + 0x30
	flush		%0 + 0x38
	subcc		%1, 0x40, %1
	bge,pt		%%icc, 1b
	 add		%2, %1, %0
"	: "=&r" (page), "=&r" (temp)
	: "r" (page), "0" (page + PAGE_SIZE - 0x40), "1" (PAGE_SIZE - 0x40)
	: "cc");
}

extern __inline__ void flush_cache_all(void)
{
	unsigned long addr;

	flushw_all();
	for(addr = 0; addr < (PAGE_SIZE << 1); addr += 32) {
		spitfire_put_icache_tag(addr, 0x0UL);
		membar("#Sync");
	}

	/* Kill the pipeline. */
	flushi(PAGE_OFFSET);
}

extern __inline__ void flush_cache_mm(struct mm_struct *mm)
{
	if(mm->context != NO_CONTEXT) {
		unsigned long addr;

		flushw_user();
		for(addr = 0; addr < (PAGE_SIZE << 1); addr += 32) {
			spitfire_put_icache_tag(addr, 0x0UL);
			membar("#Sync");
		}

		/* Kill the pipeline. */
		flushi(PAGE_OFFSET);
	}
}

extern __inline__ void flush_cache_range(struct mm_struct *mm, unsigned long start,
					 unsigned long end)
{
	if(mm->context != NO_CONTEXT) {
		unsigned long addr;

		flushw_user();
		for(addr = 0; addr < (PAGE_SIZE << 1); addr += 32) {
			spitfire_put_icache_tag(addr, 0x0UL);
			membar("#Sync");
		}

		/* Kill the pipeline. */
		flushi(PAGE_OFFSET);
	}
}

extern __inline__ void flush_cache_page(struct vm_area_struct *vma, unsigned long page)
{
	struct mm_struct *mm = vma->vm_mm;

	if(mm->context != NO_CONTEXT) {
		unsigned long addr;

		flushw_user();
		for(addr = 0; addr < (PAGE_SIZE << 1); addr += 32) {
			spitfire_put_icache_tag(addr, 0x0UL);
			membar("#Sync");
		}

		/* Kill the pipeline. */
		flushi(PAGE_OFFSET);
	}
}

/* This operation in unnecessary on the SpitFire since D-CACHE is write-through. */
#define flush_page_to_ram(page)		do { } while (0)

extern __inline__ void flush_tlb_all(void)
{
	unsigned long flags;
	int entry;

	/* Invalidate all non-locked TTE's in both the dtlb and itlb. */
	save_and_cli(flags);
	__asm__ __volatile__("stxa	%%g0, [%0] %1\n\t"
			     "stxa	%%g0, [%0] %2"
			     : /* No outputs */
			     : "r" (TLB_TAG_ACCESS), "i" (ASI_IMMU), "i" (ASI_DMMU));
	for(entry = 0; entry < 62; entry++) {
		spitfire_put_dtlb_data(entry, 0x0UL);
		spitfire_put_itlb_data(entry, 0x0UL);
	}
	membar("#Sync");
	flushi(PAGE_OFFSET);
	restore_flags(flags);
}

extern __inline__ void flush_tlb_mm(struct mm_struct *mm)
{
	if(mm->context != NO_CONTEXT) {
		unsigned long orig_ctx = spitfire_get_secondary_context();
		unsigned long flags;

		save_and_cli(flags);
		spitfire_set_secondary_context(mm->context);
		spitfire_flush_dtlb_secondary_context();
		spitfire_flush_itlb_secondary_context();
		spitfire_set_secondary_context(orig_ctx);
		restore_flags(flags);
	}
}

extern __inline__ void flush_tlb_range(struct mm_struct *mm, unsigned long start,
				       unsigned long end)
{
	if(mm->context != NO_CONTEXT) {
		unsigned long old_ctx = spitfire_get_secondary_context();
		unsigned long flags;

		start &= PAGE_MASK;
		save_and_cli(flags);
		spitfire_set_secondary_context(mm->context);
		while(start < end) {
			spitfire_flush_dtlb_secondary_page(start);
			spitfire_flush_itlb_secondary_page(start);
			start += PAGE_SIZE;
		}
		spitfire_set_secondary_context(old_ctx);
		restore_flags(flags);
	}
}

extern __inline__ void flush_tlb_page(struct vm_area_struct *vma, unsigned long page)
{
	struct mm_struct *mm = vma->vm_mm;

	if(mm->context != NO_CONTEXT) {
		unsigned long old_ctx = spitfire_get_secondary_context();
		unsigned long flags;

		page &= PAGE_MASK;
		save_and_cli(flags);
		spitfire_set_secondary_context(mm->context);
		if(vma->vm_flags & VM_EXEC)
			spitfire_flush_itlb_secondary_page(page);
		spitfire_flush_dtlb_secondary_page(page);
		spitfire_set_secondary_context(old_ctx);
		restore_flags(flags);
	}
}

extern inline pte_t mk_pte(unsigned long page, pgprot_t pgprot)
{ return __pte(__pa(page) | pgprot_val(pgprot)); }

extern inline pte_t mk_pte_phys(unsigned long physpage, pgprot_t pgprot)
{ return __pte(physpage | pgprot_val(pgprot)); }

extern inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{ pte_val(pte) = (pte_val(pte) & _PAGE_CHG_MASK) | pgprot_val(newprot); return pte; }

extern inline void pmd_set(pmd_t *pmdp, pte_t *ptep)
{ pmd_val(*pmdp) = __pa((unsigned long) ptep); }

extern inline void pgd_set(pgd_t *pgdp, pmd_t *pmdp)
{ pgd_val(*pgdp) = __pa((unsigned long) pmdp); }

extern inline unsigned long pte_page(pte_t pte)
{ return (unsigned long) __va((pte_val(pte) & _PFN_MASK)); }

extern inline unsigned long pmd_page(pmd_t pmd)
{ return (unsigned long) __va(pmd_val(pmd)); }

extern inline unsigned long pgd_page(pgd_t pgd)
{ return (unsigned long) __va(pgd_val(pgd)); }

extern inline int pte_none(pte_t pte) 		{ return !pte_val(pte); }
extern inline int pte_present(pte_t pte)	{ return pte_val(pte) & _PAGE_PRESENT; }
extern inline void pte_clear(pte_t *pte)	{ pte_val(*pte) = 0; }

extern inline int pmd_none(pmd_t pmd)		{ return pmd_val(pmd)==null_pte_table; }
extern inline int pmd_bad(pmd_t pmd)		{ return (pmd_val(pmd) & ~PAGE_MASK); }
extern inline int pmd_present(pmd_t pmd)	{ return pmd_val(pmd)!=null_pte_table; }
extern inline void pmd_clear(pmd_t *pmdp)	{ pmd_val(*pmdp) = null_pte_table; }

extern inline int pgd_none(pgd_t pgd)		{ return pgd_val(pgd)==null_pmd_table; }
extern inline int pgd_bad(pgd_t pgd)		{ return (pgd_val(pgd) & ~PAGE_MASK); }
extern inline int pgd_present(pgd_t pgd)	{ return pgd_val(pgd)!=null_pmd_table; }
extern inline void pgd_clear(pgd_t *pgdp)	{ pgd_val(*pgdp) = null_pmd_table; }

/* The following only work if pte_present() is true.
 * Undefined behaviour if not..
 */
extern inline int pte_read(pte_t pte)		{ return pte_val(pte) & _PAGE_READ; }
extern inline int pte_write(pte_t pte)		{ return pte_val(pte) & _PAGE_WRITE; }
extern inline int pte_dirty(pte_t pte)		{ return pte_val(pte) & _PAGE_MODIFIED; }
extern inline int pte_young(pte_t pte)		{ return pte_val(pte) & _PAGE_ACCESSED; }

extern inline pte_t pte_wrprotect(pte_t pte)
{ return __pte(pte_val(pte) & ~(_PAGE_WRITE|_PAGE_W)); }

extern inline pte_t pte_rdprotect(pte_t pte)
{ return __pte(pte_val(pte) & ~(_PAGE_READ|_PAGE_R)); }

extern inline pte_t pte_mkclean(pte_t pte)
{ return __pte(pte_val(pte) & ~(_PAGE_MODIFIED | _PAGE_W)); }

extern inline pte_t pte_mkold(pte_t pte)
{ return __pte(pte_val(pte) & ~(_PAGE_ACCESSED | _PAGE_R)); }

extern inline pte_t pte_mkwrite(pte_t pte)
{
	if(pte_val(pte) & _PAGE_MODIFIED)
		return __pte(pte_val(pte) | (_PAGE_WRITE | _PAGE_W));
	else
		return __pte(pte_val(pte) | (_PAGE_WRITE));
}

extern inline pte_t pte_mkdirty(pte_t pte)
{
	if(pte_val(pte) & _PAGE_WRITE)
		return __pte(pte_val(pte) | (_PAGE_MODIFIED | _PAGE_W));
	else
		return __pte(pte_val(pte) | (_PAGE_MODIFIED));
}

extern inline pte_t pte_mkyoung(pte_t pte)
{
	if(pte_val(pte) & _PAGE_READ)
		return __pte(pte_val(pte) | (_PAGE_ACCESSED | _PAGE_R));
	else
		return __pte(pte_val(pte) | (_PAGE_ACCESSED));
}

extern inline void SET_PAGE_DIR(struct task_struct *tsk, pgd_t *pgdir)
{
	register unsigned long paddr asm("o5");

	paddr = __pa(pgdir);

	if(tsk->mm == current->mm) {
		__asm__ __volatile__ ("
			rdpr		%%pstate, %%o4
			wrpr		%%o4, %1, %%pstate
			mov		%0, %%g7
			wrpr		%%o4, 0x0, %%pstate
		" : /* No outputs */
		  : "r" (paddr), "i" (PSTATE_MG|PSTATE_IE)
		  : "o4");
	}
}

/* to find an entry in a page-table-directory. */
extern inline pgd_t *pgd_offset(struct mm_struct *mm, unsigned long address)
{ return mm->pgd + ((address >> PGDIR_SHIFT) & (PTRS_PER_PAGE - 1)); }

/* to find an entry in a kernel page-table-directory */
#define pgd_offset_k(address) pgd_offset(&init_mm, address)

/* Find an entry in the second-level page table.. */
extern inline pmd_t *pmd_offset(pgd_t *dir, unsigned long address)
{ return (pmd_t *) pgd_page(*dir) + ((address >> PMD_SHIFT) & (PTRS_PER_PAGE - 1)); }

/* Find an entry in the third-level page table.. */
extern inline pte_t *pte_offset(pmd_t *dir, unsigned long address)
{ return (pte_t *) pmd_page(*dir) + ((address >> PAGE_SHIFT) & (PTRS_PER_PAGE - 1)); }

extern __inline__ void __init_pmd(pmd_t *pmdp)
{
	extern void __bfill64(void *, unsigned long);
	
	__bfill64((void *)pmdp, null_pte_table);
}

/* Allocate and free page tables. The xxx_kernel() versions are
 * used to allocate a kernel page table - this turns on supervisor
 * bits if any.
 */
extern inline void pte_free_kernel(pte_t *pte)
{ free_page((unsigned long)pte); }

extern inline pte_t * pte_alloc_kernel(pmd_t *pmd, unsigned long address)
{
	address = (address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1);
	if (pmd_none(*pmd)) {
		pte_t *page = (pte_t *) get_free_page(GFP_KERNEL);
		if (pmd_none(*pmd)) {
			if (page) {
				pmd_set(pmd, page);
				return page + address;
			}
			pmd_set(pmd, BAD_PTE);
			return NULL;
		}
		free_page((unsigned long) page);
	}
	if (pmd_bad(*pmd)) {
		printk("Bad pmd in pte_alloc_kernel: %08lx\n", pmd_val(*pmd));
		pmd_set(pmd, BAD_PTE);
		return NULL;
	}
	return (pte_t *) pmd_page(*pmd) + address;
}

extern inline void pmd_free_kernel(pmd_t *pmd)
{ free_page((unsigned long) pmd); }

extern inline pmd_t * pmd_alloc_kernel(pgd_t *pgd, unsigned long address)
{
	address = (address >> PMD_SHIFT) & (PTRS_PER_PMD - 1);
	if (pgd_none(*pgd)) {
		pmd_t *page = (pmd_t *) get_free_page(GFP_KERNEL);
		if (pgd_none(*pgd)) {
			if (page) {
				__init_pmd(page);
				pgd_set(pgd, page);
				return page + address;
			}
			pgd_set(pgd, BAD_PMD);
			return NULL;
		}
		free_page((unsigned long) page);
	}
	if (pgd_bad(*pgd)) {
		printk("Bad pgd in pmd_alloc_kernel: %08lx\n", pgd_val(*pgd));
		pgd_set(pgd, BAD_PMD);
		return NULL;
	}
	return (pmd_t *) pgd_page(*pgd) + address;
}

extern inline void pte_free(pte_t * pte)
{ free_page((unsigned long)pte); }

extern inline pte_t * pte_alloc(pmd_t *pmd, unsigned long address)
{
	address = (address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1);
	if (pmd_none(*pmd)) {
		pte_t *page = (pte_t *) get_free_page(GFP_KERNEL);
		if (pmd_none(*pmd)) {
			if (page) {
				pmd_set(pmd, page);
				return page + address;
			}
			pmd_set(pmd, BAD_PTE);
			return NULL;
		}
		free_page((unsigned long) page);
	}
	if (pmd_bad(*pmd)) {
		printk("Bad pmd in pte_alloc: %08lx\n", pmd_val(*pmd));
		pmd_set(pmd, BAD_PTE);
		return NULL;
	}
	return (pte_t *) pmd_page(*pmd) + address;
}

extern inline void pmd_free(pmd_t * pmd)
{ free_page((unsigned long) pmd); }

extern inline pmd_t * pmd_alloc(pgd_t *pgd, unsigned long address)
{
	address = (address >> PMD_SHIFT) & (PTRS_PER_PMD - 1);
	if (pgd_none(*pgd)) {
		pmd_t *page = (pmd_t *) get_free_page(GFP_KERNEL);
		if (pgd_none(*pgd)) {
			if (page) {
				__init_pmd(page);
				pgd_set(pgd, page);
				return page + address;
			}
			pgd_set(pgd, BAD_PMD);
			return NULL;
		}
		free_page((unsigned long) page);
	}
	if (pgd_bad(*pgd)) {
		printk("Bad pgd in pmd_alloc: %08lx\n", pgd_val(*pgd));
		pgd_set(pgd, BAD_PMD);
		return NULL;
	}
	return (pmd_t *) pgd_page(*pgd) + address;
}

extern inline void pgd_free(pgd_t * pgd)
{ free_page((unsigned long)pgd); }

extern inline pgd_t * pgd_alloc(void)
{
	extern void __bfill64(void *, unsigned long);
	pgd_t *pgd = (pgd_t *) __get_free_page(GFP_KERNEL);
	
	if (pgd)
		__bfill64((void *)pgd, null_pmd_table);
	return pgd;
}

extern pgd_t swapper_pg_dir[1024];

/* Routines for getting a dvma scsi buffer. */
struct mmu_sglist {
	char *addr;
	char *__dont_touch;
	unsigned int len;
	__u32 dvma_addr;
};

extern __u32 mmu_get_scsi_one(char *, unsigned long, struct linux_sbus *sbus);
extern void  mmu_get_scsi_sgl(struct mmu_sglist *, int, struct linux_sbus *sbus);

/* These do nothing with the way I have things setup. */
#define mmu_release_scsi_one(vaddr, len, sbus)	do { } while(0)
#define mmu_release_scsi_sgl(sg, sz, sbus)	do { } while(0)
#define mmu_lockarea(vaddr, len)		(vaddr)
#define mmu_unlockarea(vaddr, len)		do { } while(0)

extern inline void update_mmu_cache(struct vm_area_struct * vma,
	unsigned long address, pte_t pte)
{
	/* Find and fix bad virutal cache aliases. */
	if((vma->vm_flags & (VM_WRITE|VM_SHARED)) == (VM_WRITE|VM_SHARED)) {
		struct vm_area_struct *vmaring;
		struct inode *inode;
		unsigned long vaddr, offset, start;
		pgd_t *pgdp;
		pmd_t *pmdp;
		pte_t *ptep;
		int alias_found = 0;

		inode = vma->vm_inode;
		if(!inode)
			return;

		offset = (address & PAGE_MASK) - vma->vm_start;
		vmaring = inode->i_mmap;
		do {
			vaddr = vmaring->vm_start + offset;

			/* This conditional is misleading... */
			if((vaddr ^ address) & PAGE_SIZE) {
				alias_found++;
				start = vmaring->vm_start;
				while(start < vmaring->vm_end) {
					pgdp = pgd_offset(vmaring->vm_mm, start);
					if(!pgdp) goto next;
					pmdp = pmd_offset(pgdp, start);
					if(!pmdp) goto next;
					ptep = pte_offset(pmdp, start);
					if(!ptep) goto next;

					if(pte_val(*ptep) & _PAGE_PRESENT) {
						flush_cache_page(vmaring, start);
						*ptep = __pte(pte_val(*ptep) &
							      ~(_PAGE_CV));
						flush_tlb_page(vmaring, start);
					}
				next:
					start += PAGE_SIZE;
				}
			}
		} while((vmaring = vmaring->vm_next_share) != inode->i_mmap);

		if(alias_found && (pte_val(pte) & _PAGE_CV)) {
			pgdp = pgd_offset(vma->vm_mm, address);
			pmdp = pmd_offset(pgdp, address);
			ptep = pte_offset(pmdp, address);
			flush_cache_page(vma, address);
			*ptep = __pte(pte_val(*ptep) & ~(_PAGE_CV));
			flush_tlb_page(vma, address);
		}
	}
}

/* Make a non-present pseudo-TTE. */
extern inline pte_t mk_swap_pte(unsigned long type, unsigned long offset)
{ pte_t pte; pte_val(pte) = (type<<PAGE_SHIFT)|(offset<<(PAGE_SHIFT+8)); return pte; }

extern inline pte_t mk_pte_io(unsigned long page, pgprot_t prot, int space)
{ pte_t pte; pte_val(pte) = (page) | pgprot_val(prot); return pte; }

#define SWP_TYPE(entry)		(((entry>>PAGE_SHIFT) & 0xff))
#define SWP_OFFSET(entry)	((entry) >> (PAGE_SHIFT+8))
#define SWP_ENTRY(type,offset)	pte_val(mk_swap_pte((type),(offset)))

extern __inline__ unsigned long
sun4u_get_pte (unsigned long addr)
{
	pgd_t *pgdp;
	pmd_t *pmdp;
	pte_t *ptep;
	
	pgdp = pgd_offset (current->mm, addr);
	pmdp = pmd_offset (pgdp, addr);
	ptep = pte_offset (pmdp, addr);
	return pte_val (*ptep) & _PAGE_PADDR;
}

extern __inline__ unsigned int
__get_phys (unsigned long addr)
{
	return (sun4u_get_pte (addr) & 0x0fffffff);
}

extern __inline__ int
__get_iospace (unsigned long addr)
{
	return ((sun4u_get_pte (addr) & 0xf0000000) >> 28);
}

#endif /* !(__ASSEMBLY__) */

#endif /* !(_SPARC64_PGTABLE_H) */