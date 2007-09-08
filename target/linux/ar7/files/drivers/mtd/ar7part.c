/*
 * $Id$
 * 
 * Copyright (C) 2007 OpenWrt.org
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * TI AR7 flash partition table.
 * Based on ar7 map by Felix Fietkau.
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/bootmem.h>
#include <linux/squashfs_fs.h>

struct ar7_bin_rec {
	unsigned int checksum;
	unsigned int length;
	unsigned int address;
};

static struct mtd_partition ar7_parts[5];

static int create_mtd_partitions(struct mtd_info *master, 
				 struct mtd_partition **pparts, 
				 unsigned long origin)
{
	struct ar7_bin_rec header;
	unsigned int offset, len;
	unsigned int pre_size = master->erasesize, post_size = 0,
		root_offset = 0xe0000;
	int retries = 10;

	printk("Parsing AR7 partition map...\n");

	ar7_parts[0].name = "loader";
	ar7_parts[0].offset = 0;
	ar7_parts[0].size = master->erasesize;
	ar7_parts[0].mask_flags = MTD_WRITEABLE;

	ar7_parts[1].name = "config";
	ar7_parts[1].offset = 0;
	ar7_parts[1].size = master->erasesize;
	ar7_parts[1].mask_flags = 0;

	do {
		offset = pre_size;
		master->read(master, offset, sizeof(header), &len, (u_char *)&header);
		if (!strncmp((char *)&header, "TIENV0.8", 8))
			ar7_parts[1].offset = pre_size;
		if (header.checksum == 0xfeedfa42)
			break;
		if (header.checksum == 0xfeed1281)
			break;
		pre_size += master->erasesize;
	} while (retries--);

	pre_size = offset;

	if (!ar7_parts[1].offset) {
		ar7_parts[1].offset = master->size - master->erasesize;
		post_size = master->erasesize;
	}

	switch (header.checksum) {
	case 0xfeedfa42:
		while (header.length) {
			offset += sizeof(header) + header.length;
			master->read(master, offset, sizeof(header),
				     &len, (u_char *)&header); 
		}
		root_offset = offset + sizeof(header) + 4;
		break;
	case 0xfeed1281:
		while (header.length) {
			offset += sizeof(header) + header.length;
			master->read(master, offset, sizeof(header),
				     &len, (u_char *)&header); 
		}
		root_offset = offset + sizeof(header) + 4 + 0xff;
		root_offset &= ~(u32)0xff;
		break;
	default:
		printk("Unknown magic: %08x\n", header.checksum);
		break;
	}

	master->read(master, root_offset, sizeof(header), &len, (u_char *)&header);
	if (header.checksum != SQUASHFS_MAGIC) {
		root_offset += master->erasesize - 1;
		root_offset &= ~(master->erasesize - 1);
	}

	ar7_parts[2].name = "linux";
	ar7_parts[2].offset = pre_size;
	ar7_parts[2].size = master->size - pre_size - post_size;
	ar7_parts[2].mask_flags = 0;

	ar7_parts[3].name = "rootfs";
	ar7_parts[3].offset = root_offset;
	ar7_parts[3].size = master->size - root_offset - post_size;
	ar7_parts[3].mask_flags = 0;

	*pparts = ar7_parts;
	return 4;
}

static struct mtd_part_parser ar7_parser = {
	.owner = THIS_MODULE,
	.parse_fn = create_mtd_partitions,
	.name = "ar7part",
};

static int __init ar7_parser_init(void)
{
	return register_mtd_parser(&ar7_parser);
}

module_init(ar7_parser_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Felix Fietkau, Eugene Konev");
MODULE_DESCRIPTION("MTD partitioning for TI AR7");