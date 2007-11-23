/*
 *  linux/fs/ext/chrdev.c
 *
 *  Copyright (C) 1992  Remy Card (card@masi.ibp.fr)
 *
 *  from
 *
 *  linux/fs/minix/chrdev.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/sched.h>
#include <linux/ext_fs.h>
#include <linux/tty.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/errno.h>

/*
 * Called every time an ext character special file is opened
 */
static int chrdev_open(struct inode * inode, struct file * filp)
{
	int i;

	i = MAJOR(inode->i_rdev);
	if (i < MAX_CHRDEV) {
		filp->f_op = chrdev_fops[i];
		if (filp->f_op && filp->f_op->open)
			return filp->f_op->open(inode,filp);
	}
	return 0;
}

/*
 * Dummy default file-operations: the only thing this does
 * is contain the open that then fills in the correct operations
 * depending on the special file...
 */
static struct file_operations def_chr_fops = {
	NULL,		/* lseek */
	NULL,		/* read */
	NULL,		/* write */
	NULL,		/* readdir */
	NULL,		/* select */
	NULL,		/* ioctl */
	chrdev_open,	/* open */
	NULL,		/* release */
};

struct inode_operations ext_chrdev_inode_operations = {
	&def_chr_fops,		/* default file operations */
	NULL,			/* create */
	NULL,			/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	ext_bmap,		/* bmap */
	ext_truncate		/* truncate */
};
