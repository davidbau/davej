#ifndef __ALPHA_SYSTEM_H
#define __ALPHA_SYSTEM_H

#include <asm/pal.h>	/* for backwards compatibility... */

/*
 * System defines.. Note that this is included both from .c and .S
 * files, so it does only defines, not any C code.
 */

/*
 * We leave one page for the initial stack page, and one page for
 * the initial process structure. Also, the console eats 3 MB for
 * the initial bootloader (one of which we can reclaim later).
 * With a few other pages for various reasons, we'll use an initial
 * load address of 0xfffffc0000310000UL
 */
#define BOOT_PCB	0x20000000
#define BOOT_ADDR	0x20000000
/* Remove when official MILO sources have ELF support: */
#define BOOT_SIZE	(16*1024)

#define KERNEL_START	0xfffffc0000300000
#define SWAPPER_PGD	0xfffffc0000300000
#define INIT_STACK	0xfffffc0000302000
#define EMPTY_PGT	0xfffffc0000304000
#define EMPTY_PGE	0xfffffc0000308000
#define ZERO_PGE	0xfffffc000030A000

#define START_ADDR	0xfffffc0000310000
/* Remove when official MILO sources have ELF support: */
#define START_SIZE	(2*1024*1024)

#ifndef __ASSEMBLY__

/*
 * This is the logout header that should be common to all platforms
 * (assuming they are running OSF/1 PALcode, I guess).
 */
struct el_common {
	unsigned int	size;		/* size in bytes of logout area */
	int		sbz1	: 30;	/* should be zero */
	int		err2	:  1;	/* second error */
	int		retry	:  1;	/* retry flag */
	unsigned int	proc_offset;	/* processor-specific offset */
	unsigned int	sys_offset;	/* system-specific offset */
	unsigned long	code;		/* machine check code */
};

/* Machine Check Frame for uncorrectable errors (Large format)
 *      --- This is used to log uncorrectable errors such as
 *          double bit ECC errors.
 *      --- These errors are detected by both processor and systems.
 */
struct el_common_EV5_uncorrectable_mcheck {
        unsigned long   shadow[8];        /* Shadow reg. 8-14, 25           */
        unsigned long   paltemp[24];      /* PAL TEMP REGS.                 */
        unsigned long   exc_addr;         /* Address of excepting instruction*/
        unsigned long   exc_sum;          /* Summary of arithmetic traps.   */
        unsigned long   exc_mask;         /* Exception mask (from exc_sum). */
        unsigned long   pal_base;         /* Base address for PALcode.      */
        unsigned long   isr;              /* Interrupt Status Reg.          */
        unsigned long   icsr;             /* CURRENT SETUP OF EV5 IBOX      */
        unsigned long   ic_perr_stat;     /* I-CACHE Reg. <11> set Data parity
                                                         <12> set TAG parity*/
        unsigned long   dc_perr_stat;     /* D-CACHE error Reg. Bits set to 1:
                                                     <2> Data error in bank 0
                                                     <3> Data error in bank 1
                                                     <4> Tag error in bank 0
                                                     <5> Tag error in bank 1 */
        unsigned long   va;               /* Effective VA of fault or miss. */
        unsigned long   mm_stat;          /* Holds the reason for D-stream 
                                             fault or D-cache parity errors */
        unsigned long   sc_addr;          /* Address that was being accessed
                                             when EV5 detected Secondary cache
                                             failure.                 */
        unsigned long   sc_stat;          /* Helps determine if the error was
                                             TAG/Data parity(Secondary Cache)*/
        unsigned long   bc_tag_addr;      /* Contents of EV5 BC_TAG_ADDR    */
        unsigned long   ei_addr;          /* Physical address of any transfer
                                             that is logged in EV5 EI_STAT */
        unsigned long   fill_syndrome;    /* For correcting ECC errors.     */
        unsigned long   ei_stat;          /* Helps identify reason of any 
                                             processor uncorrectable error
                                             at its external interface.     */
        unsigned long   ld_lock;          /* Contents of EV5 LD_LOCK register*/
};


extern void wrent(void *, unsigned long);
extern void wrkgp(unsigned long);
extern void wrusp(unsigned long);
extern unsigned long rdusp(void);
extern unsigned long rdmces (void);
extern void wrmces (unsigned long);
extern unsigned long whami(void);
extern void wripir(unsigned long);

#define halt() __asm__ __volatile__ ("call_pal %0" : : "i" (PAL_halt) : "memory")

#define switch_to(prev,next) do { \
	current = next; \
	alpha_switch_to((unsigned long) &current->tss - 0xfffffc0000000000); \
} while (0)

extern void alpha_switch_to(unsigned long pctxp);

#define mb() \
__asm__ __volatile__("mb": : :"memory")

#define imb() \
__asm__ __volatile__ ("call_pal %0" : : "i" (PAL_imb) : "memory")

#define draina() \
__asm__ __volatile__ ("call_pal %0" : : "i" (PAL_draina) : "memory")

#define call_pal1(palno,arg) \
({ \
	register unsigned long __r0 __asm__("$0"); \
	register unsigned long __r16 __asm__("$16"); __r16 = arg; \
	__asm__ __volatile__( \
		"call_pal %3" \
		:"=r" (__r0),"=r" (__r16) \
		:"1" (__r16),"i" (palno) \
		:"$1", "$22", "$23", "$24", "$25", "memory"); \
	__r0; \
})

#define getipl() \
({ \
	register unsigned long r0 __asm__("$0"); \
	__asm__ __volatile__( \
		"call_pal %1" \
		:"=r" (r0) \
		:"i" (PAL_rdps) \
		:"$1", "$16", "$22", "$23", "$24", "$25", "memory"); \
	r0; \
})

#ifdef THE_OLD_VERSION
#define setipl(ipl) \
do { \
	register unsigned long __r16 __asm__("$16") = (ipl); \
	__asm__ __volatile__( \
		"call_pal %2" \
		:"=r" (__r16) \
		:"0" (__r16),"i" (PAL_swpipl) \
		:"$0", "$1", "$22", "$23", "$24", "$25", "memory"); \
} while (0)

#define swpipl(ipl) \
({ \
	register unsigned long __r0 __asm__("$0"); \
	register unsigned long __r16 __asm__("$16") = (ipl); \
	__asm__ __volatile__( \
		"call_pal %3" \
		:"=r" (__r0),"=r" (__r16) \
		:"1" (__r16),"i" (PAL_swpipl) \
		:"$1", "$22", "$23", "$24", "$25", "memory"); \
	__r0; \
})
#else
#define setipl(ipl) \
do { \
	__asm__ __volatile__( \
		"mov %0,$16; call_pal %1" \
		: /* no output */ \
		:"i,r" (ipl), "i,i" (PAL_swpipl) \
		:"$0", "$1", "$16", "$22", "$23", "$24", "$25", "memory"); \
} while (0)

#define swpipl(ipl) \
({ \
	register unsigned long __r0 __asm__("$0"); \
	__asm__ __volatile__( \
		"mov %0,$16; call_pal %1" \
		: /* no output (bound to the template) */ \
		: "i,r" (ipl), "i,i" (PAL_swpipl) \
		: "$0", "$1", "$16", "$22", "$23", "$24", "$25", "memory"); \
	__r0; \
})
#endif

#define __cli()			setipl(7)
#define __sti()			setipl(0)
#define __save_flags(flags)	do { (flags) = getipl(); } while (0)
#define __save_and_cli(flags)	do { (flags) = swpipl(7); } while (0)
#define __restore_flags(flags)	setipl(flags)

#ifdef __SMP__

extern unsigned char global_irq_holder;

#define save_flags(x) \
do { \
	(x) = ((global_irq_holder == (unsigned char) smp_processor_id()) \
		? 1 \
		: ((getipl() & 7) ? 2 : 0)); \
} while (0)

#define save_and_cli(flags)   do { save_flags(flags); cli(); } while(0)

extern void __global_cli(void);
extern void __global_sti(void);
extern void __global_restore_flags(unsigned long flags);

#define cli()                   __global_cli()
#define sti()                   __global_sti()
#define restore_flags(flags)    __global_restore_flags(flags)

#else /* __SMP__ */

#define cli()			setipl(7)
#define sti()			setipl(0)
#define save_flags(flags)	do { (flags) = getipl(); } while (0)
#define save_and_cli(flags)	do { (flags) = swpipl(7); } while (0)
#define restore_flags(flags)	setipl(flags)

#endif /* __SMP__ */

/*
 * TB routines..
 */
#define __tbi(nr,arg,arg1...) do { \
	register unsigned long __r16 __asm__("$16") = (nr); \
	register unsigned long __r17 __asm__("$17"); arg; \
	__asm__ __volatile__( \
		"call_pal %3" \
		:"=r" (__r16),"=r" (__r17) \
		:"0" (__r16),"i" (PAL_tbi) ,##arg1 \
		:"$0", "$1", "$22", "$23", "$24", "$25"); \
} while (0)

#define tbi(x,y)	__tbi(x,__r17=(y),"1" (__r17))
#define tbisi(x)	__tbi(1,__r17=(x),"1" (__r17))
#define tbisd(x)	__tbi(2,__r17=(x),"1" (__r17))
#define tbis(x)		__tbi(3,__r17=(x),"1" (__r17))
#define tbiap()		__tbi(-1, /* no second argument */)
#define tbia()		__tbi(-2, /* no second argument */)

/*
 * Give prototypes to shut up gcc.
 */
extern __inline__ unsigned long xchg_u32 (volatile int * m, unsigned long val);
extern __inline__ unsigned long xchg_u64 (volatile long * m, unsigned long val);

extern __inline__ unsigned long xchg_u32(volatile int * m, unsigned long val)
{
	unsigned long dummy;

	__asm__ __volatile__(
	"1:	ldl_l %0,%2\n"
	"	bis %3,%3,%1\n"
	"	stl_c %1,%2\n"
	"	beq %1,2f\n"
	".section .text2,\"ax\"\n"
	"2:	br 1b\n"
	".previous"
	: "=&r" (val), "=&r" (dummy), "=m" (*m)
	: "r" (val), "m" (*m));

	return val;
}

extern __inline__ unsigned long xchg_u64(volatile long * m, unsigned long val)
{
	unsigned long dummy;

	__asm__ __volatile__(
	"1:	ldq_l %0,%2\n"
	"	bis %3,%3,%1\n"
	"	stq_c %1,%2\n"
	"	beq %1,2f\n"
	".section .text2,\"ax\"\n"
	"2:	br 1b\n"
	".previous"
	: "=&r" (val), "=&r" (dummy), "=m" (*m)
	: "r" (val), "m" (*m));

	return val;
}

#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))
#define tas(ptr) (xchg((ptr),1))

/*
 * This function doesn't exist, so you'll get a linker error
 * if something tries to do an invalid xchg().
 *
 * This only works if the compiler isn't horribly bad at optimizing.
 * gcc-2.5.8 reportedly can't handle this, but as that doesn't work
 * too well on the alpha anyway..
 */
extern void __xchg_called_with_bad_pointer(void);

static __inline__ unsigned long __xchg(unsigned long x, volatile void * ptr, int size)
{
	switch (size) {
		case 4:
			return xchg_u32(ptr, x);
		case 8:
			return xchg_u64(ptr, x);
	}
	__xchg_called_with_bad_pointer();
	return x;
}

#endif /* __ASSEMBLY__ */

#endif
