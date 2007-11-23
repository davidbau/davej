/* $Id: mmu_context.h,v 1.36 1999/05/25 16:53:34 jj Exp $ */
#ifndef __SPARC64_MMU_CONTEXT_H
#define __SPARC64_MMU_CONTEXT_H

/* Derived heavily from Linus's Alpha/AXP ASN code... */

#include <asm/system.h>
#include <asm/spitfire.h>
#include <asm/spinlock.h>

#define NO_CONTEXT     0

#ifndef __ASSEMBLY__

extern unsigned long tlb_context_cache;
extern unsigned long mmu_context_bmap[];

#define CTX_VERSION_SHIFT	(PAGE_SHIFT - 3)
#define CTX_VERSION_MASK	((~0UL) << CTX_VERSION_SHIFT)
#define CTX_FIRST_VERSION	((1UL << CTX_VERSION_SHIFT) + 1UL)

extern void get_new_mmu_context(struct mm_struct *mm);

/* Initialize/destroy the context related info for a new mm_struct
 * instance.
 */
#define init_new_context(__mm)	((__mm)->context = NO_CONTEXT)

/* Kernel threads like rpciod and nfsd drop their mm, and then use
 * init_mm, when this happens we must make sure the secondary context is
 * updated as well.  Otherwise we have disasters relating to
 * set_fs/get_fs usage later on.
 *
 * Also we can only clear the mmu_context_bmap bit when this is
 * the final reference to the address space.
 */
#define destroy_context(__mm)	do { 						\
	if ((__mm)->context != NO_CONTEXT &&					\
	    atomic_read(&(__mm)->count) == 1) { 				\
		if (!(((__mm)->context ^ tlb_context_cache) & CTX_VERSION_MASK))\
			clear_bit((__mm)->context & ~(CTX_VERSION_MASK),	\
				  mmu_context_bmap);				\
		(__mm)->context = NO_CONTEXT; 					\
		if(current->mm == (__mm)) {					\
			current->tss.ctx = 0;					\
			spitfire_set_secondary_context(0);			\
			__asm__ __volatile__("flush %g6");			\
		}								\
	} 									\
} while (0)

/* The caller must flush the current set of user windows
 * to the stack (if necessary) before we get here.
 */
extern __inline__ void __get_mmu_context(struct task_struct *tsk)
{
	register unsigned long paddr asm("o5");
	register unsigned long pgd_cache asm("o4");
	struct mm_struct *mm = tsk->mm;
	unsigned long asi;

	if(!(tsk->tss.flags & SPARC_FLAG_KTHREAD)	&&
	   !(tsk->flags & PF_EXITING)) {
		unsigned long ctx = tlb_context_cache;
		if((mm->context ^ ctx) & CTX_VERSION_MASK)
			get_new_mmu_context(mm);
		tsk->tss.ctx = mm->context & 0x3ff;
		spitfire_set_secondary_context(mm->context & 0x3ff);
		__asm__ __volatile__("flush %g6");
		if(!(mm->cpu_vm_mask & (1UL<<smp_processor_id()))) {
			spitfire_flush_dtlb_secondary_context();
			spitfire_flush_itlb_secondary_context();
			__asm__ __volatile__("flush %g6");
		}
		asi = tsk->tss.current_ds.seg;
	} else {
		tsk->tss.ctx = 0;
		spitfire_set_secondary_context(0);
		__asm__ __volatile__("flush %g6");
		asi = ASI_P;
	}
	/* Sigh, damned include loops... just poke seg directly.  */
	__asm__ __volatile__ ("wr %%g0, %0, %%asi" : : "r" (asi));
	paddr = __pa(mm->pgd);
	if((tsk->tss.flags & (SPARC_FLAG_32BIT|SPARC_FLAG_KTHREAD)) ==
	   (SPARC_FLAG_32BIT))
		pgd_cache = ((unsigned long) mm->pgd[0]) << 11UL;
	else
		pgd_cache = 0;
	__asm__ __volatile__("
		rdpr		%%pstate, %%o2
		andn		%%o2, %2, %%o3
		wrpr		%%o3, %5, %%pstate
		mov		%4, %%g4
		mov		%0, %%g7
		stxa		%1, [%%g4] %3
		wrpr		%%o2, 0x0, %%pstate
	" : /* no outputs */
	  : "r" (paddr), "r" (pgd_cache), "i" (PSTATE_IE),
	    "i" (ASI_DMMU), "i" (TSB_REG), "i" (PSTATE_MG)
	  : "o2", "o3");
}

/* Now we define this as a do nothing macro, because the only
 * generic user right now is the scheduler, and we handle all
 * the atomicity issues by having switch_to() call the above
 * function itself.
 */
#define get_mmu_context(x)	do { } while(0)

/*
 * After we have set current->mm to a new value, this activates
 * the context for the new mm so we see the new mappings.  Currently,
 * this is always called for 'current', if that changes put appropriate
 * checks here.
 *
 * We set the cpu_vm_mask first to zero to enforce a tlb flush for
 * the new context above, then we set it to the current cpu so the
 * smp tlb flush routines do not get confused.
 */
#define activate_context(__tsk)		\
do {	flushw_user();			\
	(__tsk)->mm->cpu_vm_mask = 0;	\
	__get_mmu_context(__tsk);	\
	(__tsk)->mm->cpu_vm_mask = (1UL<<smp_processor_id()); \
} while(0)

#endif /* !(__ASSEMBLY__) */

#endif /* !(__SPARC64_MMU_CONTEXT_H) */
