#
# ia64/boot/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1998 by David Mosberger-Tang <davidm@hpl.hp.com>
#

LINKFLAGS = -static -T bootloader.lds

.S.s:
	$(CPP) $(AFLAGS) -traditional -o $*.o $<
.S.o:
	$(CC) $(AFLAGS) -traditional -c -o $*.o $<

OBJECTS	= bootloader.o
TARGETS =

ifdef CONFIG_IA64_HP_SIM
 TARGETS += bootloader
endif

all:	$(TARGETS)

bootloader: $(OBJECTS)
	$(LD) $(LINKFLAGS) $(OBJECTS) $(TOPDIR)/lib/lib.a $(TOPDIR)/arch/$(ARCH)/lib/lib.a \
	-o bootloader

clean:
	rm -f $(TARGETS)

dep:
