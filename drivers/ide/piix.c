/*
 * linux/drivers/block/piix.c		Version 0.30	Feb. 26, 2000
 *
 *  Copyright (C) 1998-1999 Andrzej Krzysztofowicz, Author and Maintainer
 *  Copyright (C) 1998-2000 Andre Hedrick (andre@suse.com)
 *  May be copied or modified under the terms of the GNU General Public License
 *
 *  PIO mode setting function for Intel chipsets.  
 *  For use instead of BIOS settings.
 *
 * 40-41
 * 42-43
 * 
 *                 41
 *                 43
 *
 * | PIO 0       | c0 | 80 | 0 | 	piix_tune_drive(drive, 0);
 * | PIO 2 | SW2 | d0 | 90 | 4 | 	piix_tune_drive(drive, 2);
 * | PIO 3 | MW1 | e1 | a1 | 9 | 	piix_tune_drive(drive, 3);
 * | PIO 4 | MW2 | e3 | a3 | b | 	piix_tune_drive(drive, 4);
 * 
 * sitre = word40 & 0x4000; primary
 * sitre = word42 & 0x4000; secondary
 *
 * 44 8421|8421    hdd|hdb
 * 
 * 48 8421         hdd|hdc|hdb|hda udma enabled
 *
 *    0001         hda
 *    0010         hdb
 *    0100         hdc
 *    1000         hdd
 *
 * 4a 84|21        hdb|hda
 * 4b 84|21        hdd|hdc
 *
 *    ata-33/82371AB
 *    ata-33/82371EB
 *    ata-33/82801AB            ata-66/82801AA
 *    00|00 udma 0              00|00 reserved
 *    01|01 udma 1              01|01 udma 3
 *    10|10 udma 2              10|10 udma 4
 *    11|11 reserved            11|11 reserved
 *
 * 54 8421|8421    ata66 drive|ata66 enable
 *
 * pci_read_config_word(HWIF(drive)->pci_dev, 0x40, &reg40);
 * pci_read_config_word(HWIF(drive)->pci_dev, 0x42, &reg42);
 * pci_read_config_word(HWIF(drive)->pci_dev, 0x44, &reg44);
 * pci_read_config_word(HWIF(drive)->pci_dev, 0x48, &reg48);
 * pci_read_config_word(HWIF(drive)->pci_dev, 0x4a, &reg4a);
 * pci_read_config_word(HWIF(drive)->pci_dev, 0x54, &reg54);
 *
 * 00:1f.1 IDE interface: Intel Corporation:
 *                          Unknown device 2411 (rev 01) (prog-if 80 [Master])
 *         Control: I/O+ Mem- BusMaster+ SpecCycle- MemWINV- VGASnoop-
 *                          ParErr- Stepping- SERR- FastB2B-
 *         Status: Cap- 66Mhz- UDF- FastB2B+ ParErr- DEVSEL=medium >TAbort-
 *                          <TAbort- <MAbort- >SERR- <PERR-
 *         Latency: 0 set
 *         Region 4: I/O ports at ffa0
 * 00: 86 80 11 24 05 00 80 02 01 80 01 01 00 00 00 00
 * 10: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 20: a1 ff 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 30: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 40: 07 a3 03 a3 00 00 00 00 05 00 02 02 00 00 00 00
 * 50: 00 00 00 00 11 04 00 00 00 00 00 00 00 00 00 00
 * 60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 90: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * a0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * b0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * d0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * f0: 00 00 00 00 00 00 00 00 3a 0f 00 00 00 00 00 00
 *
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/hdreg.h>
#include <linux/ide.h>
#include <linux/delay.h>

#include <asm/io.h>

#include "ide_modes.h"

#define PIIX_DEBUG_DRIVE_INFO		0

#define DISPLAY_PIIX_TIMINGS

#if defined(DISPLAY_PIIX_TIMINGS) && defined(CONFIG_PROC_FS)
#include <linux/stat.h>
#include <linux/proc_fs.h>

static int piix_get_info(char *, char **, off_t, int);
extern int (*piix_display_info)(char *, char **, off_t, int); /* ide-proc.c */
extern char *ide_media_verbose(ide_drive_t *);
static struct pci_dev *bmide_dev;

static int piix_get_info (char *buffer, char **addr, off_t offset, int count)
{
	char *p = buffer;
	u32 bibma = bmide_dev->resource[4].start;
        u16 reg40 = 0, psitre = 0, reg42 = 0, ssitre = 0;
	u8  c0 = 0, c1 = 0;
	u8  reg44 = 0, reg48 = 0, reg4a = 0, reg4b = 0, reg54 = 0;

	pci_read_config_word(bmide_dev, 0x40, &reg40);
	pci_read_config_word(bmide_dev, 0x42, &reg42);
	pci_read_config_byte(bmide_dev, 0x44, &reg44);
	pci_read_config_byte(bmide_dev, 0x48, &reg48);
	pci_read_config_byte(bmide_dev, 0x4a, &reg4a);
	pci_read_config_byte(bmide_dev, 0x4b, &reg4b);
	pci_read_config_byte(bmide_dev, 0x54, &reg54);

	psitre = (reg40 & 0x4000) ? 1 : 0;
	ssitre = (reg42 & 0x4000) ? 1 : 0;

        /*
         * at that point bibma+0x2 et bibma+0xa are byte registers
         * to investigate:
         */
	c0 = inb_p((unsigned short)bibma + 0x02);
	c1 = inb_p((unsigned short)bibma + 0x0a);

	switch(bmide_dev->device) {
		case PCI_DEVICE_ID_INTEL_82372FB_1:
		case PCI_DEVICE_ID_INTEL_82801AA_1:
			p += sprintf(p, "\n                                Intel PIIX4 Ultra 66 Chipset.\n");
			break;
		case PCI_DEVICE_ID_INTEL_82801AB_1:
		case PCI_DEVICE_ID_INTEL_82371AB:
			p += sprintf(p, "\n                                Intel PIIX4 Ultra 33 Chipset.\n");
			break;
		case PCI_DEVICE_ID_INTEL_82371SB_1:
			p += sprintf(p, "\n                                Intel PIIX3 Chipset.\n");
			break;
		case PCI_DEVICE_ID_INTEL_82371FB_1:
		case PCI_DEVICE_ID_INTEL_82371FB_0:
		default:
			p += sprintf(p, "\n                                Intel PIIX Chipset.\n");
			break;
	}
	p += sprintf(p, "--------------- Primary Channel ---------------- Secondary Channel -------------\n");
	p += sprintf(p, "                %sabled                         %sabled\n",
			(c0&0x80) ? "dis" : " en",
			(c1&0x80) ? "dis" : " en");
	p += sprintf(p, "--------------- drive0 --------- drive1 -------- drive0 ---------- drive1 ------\n");
	p += sprintf(p, "DMA enabled:    %s              %s             %s               %s\n",
			(c0&0x20) ? "yes" : "no ",
			(c0&0x40) ? "yes" : "no ",
			(c1&0x20) ? "yes" : "no ",
			(c1&0x40) ? "yes" : "no " );
	p += sprintf(p, "UDMA enabled:   %s              %s             %s               %s\n",
			(reg48&0x01) ? "yes" : "no ",
			(reg48&0x02) ? "yes" : "no ",
			(reg48&0x04) ? "yes" : "no ",
			(reg48&0x08) ? "yes" : "no " );
	p += sprintf(p, "UDMA enabled:   %s                %s               %s                 %s\n",
			((reg54&0x11) && (reg4a&0x02)) ? "4" :
			((reg54&0x11) && (reg4a&0x01)) ? "3" :
			(reg4a&0x02) ? "2" :
			(reg4a&0x01) ? "1" :
			(reg4a&0x00) ? "0" : "X",
			((reg54&0x22) && (reg4a&0x20)) ? "4" :
			((reg54&0x22) && (reg4a&0x10)) ? "3" :
			(reg4a&0x20) ? "2" :
			(reg4a&0x10) ? "1" :
			(reg4a&0x00) ? "0" : "X",
			((reg54&0x44) && (reg4b&0x02)) ? "4" :
			((reg54&0x44) && (reg4b&0x01)) ? "3" :
			(reg4b&0x02) ? "2" :
			(reg4b&0x01) ? "1" :
			(reg4b&0x00) ? "0" : "X",
			((reg54&0x88) && (reg4b&0x20)) ? "4" :
			((reg54&0x88) && (reg4b&0x10)) ? "3" :
			(reg4b&0x20) ? "2" :
			(reg4b&0x10) ? "1" :
			(reg4b&0x00) ? "0" : "X");

	p += sprintf(p, "UDMA\n");
	p += sprintf(p, "DMA\n");
	p += sprintf(p, "PIO\n");

/*
 *	FIXME.... Add configuration junk data....blah blah......
 */

	return p-buffer;	 /* => must be less than 4k! */
}
#endif  /* defined(DISPLAY_PIIX_TIMINGS) && defined(CONFIG_PROC_FS) */

/*
 *  Used to set Fifo configuration via kernel command line:
 */

byte piix_proc = 0;

extern char *ide_xfer_verbose (byte xfer_rate);

/*
 *
 */
static byte piix_dma_2_pio (byte xfer_rate) {
	switch(xfer_rate) {
		case XFER_UDMA_4:
		case XFER_UDMA_3:
		case XFER_UDMA_2:
		case XFER_UDMA_1:
		case XFER_UDMA_0:
		case XFER_MW_DMA_2:
		case XFER_PIO_4:
			return 4;
		case XFER_MW_DMA_1:
		case XFER_PIO_3:
			return 3;
		case XFER_SW_DMA_2:
		case XFER_PIO_2:
			return 2;
		case XFER_MW_DMA_0:
		case XFER_SW_DMA_1:
		case XFER_SW_DMA_0:
		case XFER_PIO_1:
		case XFER_PIO_0:
		case XFER_PIO_SLOW:
		default:
			return 0;
	}
}

/*
 *  Based on settings done by AMI BIOS
 *  (might be usefull if drive is not registered in CMOS for any reason).
 */
static void piix_tune_drive (ide_drive_t *drive, byte pio)
{
	unsigned long flags;
	u16 master_data;
	byte slave_data;
	int is_slave		= (&HWIF(drive)->drives[1] == drive);
	int master_port		= HWIF(drive)->index ? 0x42 : 0x40;
	int slave_port		= 0x44;
				 /* ISP  RTC */
	byte timings[][2]	= { { 0, 0 },
				    { 0, 0 },
				    { 1, 0 },
				    { 2, 1 },
				    { 2, 3 }, };

	pio = ide_get_best_pio_mode(drive, pio, 5, NULL);
	pci_read_config_word(HWIF(drive)->pci_dev, master_port, &master_data);
	if (is_slave) {
		master_data = master_data | 0x4000;
		if (pio > 1)
			/* enable PPE, IE and TIME */
			master_data = master_data | 0x0070;
		pci_read_config_byte(HWIF(drive)->pci_dev, slave_port, &slave_data);
		slave_data = slave_data & (HWIF(drive)->index ? 0x0f : 0xf0);
		slave_data = slave_data | ((timings[pio][0] << 2) | (timings[pio][1]
					   << (HWIF(drive)->index ? 4 : 0)));
	} else {
		master_data = master_data & 0xccf8;
		if (pio > 1)
			/* enable PPE, IE and TIME */
			master_data = master_data | 0x0007;
		master_data = master_data | (timings[pio][0] << 12) |
			      (timings[pio][1] << 8);
	}
	save_flags(flags);
	cli();
	pci_write_config_word(HWIF(drive)->pci_dev, master_port, master_data);
	if (is_slave)
		pci_write_config_byte(HWIF(drive)->pci_dev, slave_port, slave_data);
	restore_flags(flags);
}

static int piix_config_drive_for_dma (ide_drive_t *drive)
{
	struct hd_driveid *id	= drive->id;
	ide_hwif_t *hwif	= HWIF(drive);
	struct pci_dev *dev	= hwif->pci_dev;

	int			sitre;
	short			reg4042, reg44, reg48, reg4a, reg54;
	byte			speed;

	byte maslave		= hwif->channel ? 0x42 : 0x40;
	byte udma_66		= ((id->hw_config & 0x2000) && (hwif->udma_four)) ? 1 : 0;
	int ultra66		= ((dev->device == PCI_DEVICE_ID_INTEL_82801AA_1) ||
				   (dev->device == PCI_DEVICE_ID_INTEL_82372FB_1)) ? 1 : 0;
	int ultra		= ((ultra66) ||
				   (dev->device == PCI_DEVICE_ID_INTEL_82371AB) ||
				   (dev->device == PCI_DEVICE_ID_INTEL_82801AB_1)) ? 1 : 0;
	int drive_number	= ((hwif->channel ? 2 : 0) + (drive->select.b.unit & 0x01));
	int a_speed		= 2 << (drive_number * 4);
	int u_flag		= 1 << drive_number;
	int v_flag		= 0x10 << drive_number;
	int u_speed		= 0;

	pci_read_config_word(dev, maslave, &reg4042);
	sitre = (reg4042 & 0x4000) ? 1 : 0;
	pci_read_config_word(dev, 0x44, &reg44);
	pci_read_config_word(dev, 0x48, &reg48);
	pci_read_config_word(dev, 0x4a, &reg4a);
	pci_read_config_word(dev, 0x54, &reg54);

	if ((id->dma_ultra & 0x0010) && (ultra)) {
		u_speed = 2 << (drive_number * 4);
		speed = ((udma_66) && (ultra66)) ? XFER_UDMA_4 : XFER_UDMA_2;
	} else if ((id->dma_ultra & 0x0008) && (ultra)) {
		u_speed = 1 << (drive_number * 4);
		speed = ((udma_66) && (ultra66)) ? XFER_UDMA_3 : XFER_UDMA_1;
	} else if ((id->dma_ultra & 0x0004) && (ultra)) {
		u_speed = 2 << (drive_number * 4);
		speed = XFER_UDMA_2;
	} else if ((id->dma_ultra & 0x0002) && (ultra)) {
		u_speed = 1 << (drive_number * 4);
		speed = XFER_UDMA_1;
	} else if ((id->dma_ultra & 0x0001) && (ultra)) {
		u_speed = 0 << (drive_number * 4);
		speed = XFER_UDMA_0;
	} else if (id->dma_mword & 0x0004) {
		speed = XFER_MW_DMA_2;
	} else if (id->dma_mword & 0x0002) {
		speed = XFER_MW_DMA_1;
	} else if (id->dma_1word & 0x0004) {
		speed = XFER_SW_DMA_2;
        } else {
		speed = XFER_PIO_0 + ide_get_best_pio_mode(drive, 255, 5, NULL);
	}

	if (speed >= XFER_UDMA_0) {
		if (!(reg48 & u_flag))
			pci_write_config_word(dev, 0x48, reg48|u_flag);
		if (!(reg4a & u_speed)) {
			pci_write_config_word(dev, 0x4a, reg4a & ~a_speed);
			pci_write_config_word(dev, 0x4a, reg4a|u_speed);
		}
		if (speed > XFER_UDMA_2) {
			if (!(reg54 & v_flag)) {
				pci_write_config_word(dev, 0x54, reg54|v_flag);
			}
		} else {
			pci_write_config_word(dev, 0x54, reg54 & ~v_flag);
		}
	}

	if (speed < XFER_UDMA_0) {
		if (reg48 & u_flag)
			pci_write_config_word(dev, 0x48, reg48 & ~u_flag);
		if (reg4a & a_speed)
			pci_write_config_word(dev, 0x4a, reg4a & ~a_speed);
		if (reg54 & v_flag)
			pci_write_config_word(dev, 0x54, reg54 & ~v_flag);
	}

	piix_tune_drive(drive, piix_dma_2_pio(speed));

	(void) ide_config_drive_speed(drive, speed);

#if PIIX_DEBUG_DRIVE_INFO
	printk("%s: %s drive%d\n", drive->name, ide_xfer_verbose(speed), drive_number);
#endif /* PIIX_DEBUG_DRIVE_INFO */

	return ((int)	((id->dma_ultra >> 11) & 3) ? ide_dma_on :
			((id->dma_ultra >> 8) & 7) ? ide_dma_on :
			((id->dma_mword >> 8) & 7) ? ide_dma_on :
			((id->dma_1word >> 8) & 7) ? ide_dma_on :
						     ide_dma_off_quietly);
}

static int piix_dmaproc(ide_dma_action_t func, ide_drive_t *drive)
{
	switch (func) {
		case ide_dma_check:
			 return ide_dmaproc((ide_dma_action_t) piix_config_drive_for_dma(drive), drive);
		default :
			break;
	}
	/* Other cases are done by generic IDE-DMA code. */
	return ide_dmaproc(func, drive);
}

unsigned int __init pci_init_piix (struct pci_dev *dev, const char *name)
{
#if defined(DISPLAY_PIIX_TIMINGS) && defined(CONFIG_PROC_FS)
	piix_proc = 1;
	bmide_dev = dev;
	piix_display_info = &piix_get_info;
#endif /* DISPLAY_PIIX_TIMINGS && CONFIG_PROC_FS */
	return 0;
}

/*
 * Sheesh, someone at Intel needs to go read the ATA-4/5 T13 standards.
 * It does not specify device detection, but channel!!!
 * You determine later if bit 13 of word93 is set...
 */
unsigned int __init ata66_piix (ide_hwif_t *hwif)
{
	byte reg54h = 0, reg55h = 0, ata66 = 0;
	byte mask = hwif->channel ? 0x0c : 0x03;

	pci_read_config_byte(hwif->pci_dev, 0x54, &reg54h);
	pci_read_config_byte(hwif->pci_dev, 0x55, &reg55h);
	ata66 = (reg54h & mask) ? 1 : 0;

	return ata66;
}

void __init ide_init_piix (ide_hwif_t *hwif)
{
	hwif->tuneproc = &piix_tune_drive;

	if (hwif->dma_base) {
#ifdef CONFIG_PIIX_TUNING
		hwif->dmaproc = &piix_dmaproc;
#endif /* CONFIG_PIIX_TUNING */
		hwif->drives[0].autotune = 0;
		hwif->drives[1].autotune = 0;
	} else {
		hwif->drives[0].autotune = 1;
		hwif->drives[1].autotune = 1;
	}
	if (!hwif->irq)
		hwif->irq = hwif->channel ? 15 : 14;
}