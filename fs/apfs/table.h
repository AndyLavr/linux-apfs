/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  linux/fs/apfs/table.h
 *
 * Copyright (C) 2018 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _APFS_TABLE_H
#define _APFS_TABLE_H

#include <linux/fs.h>
#include <linux/kref.h>
#include <linux/types.h>
#include "apfs.h"
#include "btree.h"

/*
 * On-disk representation of an object map
 */
struct apfs_omap_phys {
/*00*/	struct apfs_obj_phys om_o;
/*20*/	__le32 om_flags;
	__le32 om_snap_count;
	__le32 om_tree_type;
	__le32 om_snapshot_tree_type;
/*30*/	__le64 om_tree_oid;
	__le64 om_snapshot_tree_oid;
/*40*/	__le64 om_most_recent_snap;
	__le64 om_pending_revert_min;
	__le64 om_pending_revert_max;
} __packed;

/*
 * Structure of a value in an object map B-tree
 */
struct apfs_omap_val {
	__le32 ov_flags;
	__le32 ov_size;
	__le64 ov_paddr;
} __packed;

/* B-tree node flags */
#define APFS_BTNODE_ROOT		0x0001
#define APFS_BTNODE_LEAF		0x0002
#define APFS_BTNODE_FIXED_KV_SIZE	0x0004
#define APFS_BTNODE_CHECK_KOFF_INVAL	0x8000

/* B-tree location constants */
#define APFS_BTOFF_INVALID		0xffff

/*
 * Structure storing a location inside a B-tree node
 */
struct apfs_nloc {
	__le16 off;
	__le16 len;
} __packed;

/*
 * Structure storing the location of a key/value pair within a B-tree node
 */
struct apfs_kvloc {
	struct apfs_nloc k;
	struct apfs_nloc v;
} __packed;

/*
 * Structure storing the location of a key/value pair within a B-tree node
 * having fixed-size key and value (flag APFS_BTNODE_FIXED_KV_SIZE is present)
 */
struct apfs_kvoff {
	__le16 k;
	__le16 v;
} __packed;

/*
 * On-disk representation of a B-tree node
 */
struct apfs_btree_node_phys {
/*00*/	struct apfs_obj_phys btn_o;
/*20*/	__le16 btn_flags;
	__le16 btn_level;
	__le32 btn_nkeys;
/*28*/	struct apfs_nloc btn_table_space;
	struct apfs_nloc btn_free_space;
	struct apfs_nloc btn_key_free_list;
	struct apfs_nloc btn_val_free_list;
/*38*/	__le64 btn_data[];
} __packed;

/*
 * Structure used to store information about a B-tree that won't change
 * over time
 */
struct apfs_btree_info_fixed {
	__le32 bt_flags;
	__le32 bt_node_size;
	__le32 bt_key_size;
	__le32 bt_val_size;
} __packed;

/*
 * Structure used to store information about a B-tree (located at the end of
 * a B-tree root node block)
 */
struct apfs_btree_info {
	struct apfs_btree_info_fixed bt_fixed;
	__le32 bt_longest_key;
	__le32 bt_longest_val;
	__le64 bt_key_count;
	__le64 bt_node_count;
} __packed;

/*
 * In-memory representation of an APFS table
 */
struct apfs_table {
	u16 t_flags;		/* Table flags */
	u16 t_records;		/* Number of records in the table */

	int t_key;		/* Offset of the key area in the block */
	int t_free;		/* Offset of the free area in the block */
	int t_data;		/* Offset of the data area in the block */

	struct apfs_node t_node;/* Node holding the table */

	struct kref refcount;
};

/**
 * apfs_table_is_leaf - Check if a b-tree table is a leaf node
 * @table: the table to check
 */
static inline bool apfs_table_is_leaf(struct apfs_table *table)
{
	return (table->t_flags & APFS_BTNODE_LEAF) != 0;
}

/**
 * apfs_table_is_root - Check if a b-tree table is a root node
 * @table: the table to check
 */
static inline bool apfs_table_is_root(struct apfs_table *table)
{
	return (table->t_flags & APFS_BTNODE_ROOT) != 0;
}

/**
 * apfs_table_has_fixed_kv_size - Check if a b-tree table has fixed key/value
 * sizes
 * @table: the table to check
 */
static inline bool apfs_table_has_fixed_kv_size(struct apfs_table *table)
{
	return (table->t_flags & APFS_BTNODE_FIXED_KV_SIZE) != 0;
}

extern struct apfs_table *apfs_read_table(struct super_block *sb, u64 block);
extern int apfs_table_locate_key(struct apfs_table *table,
				 int index, int *off);
extern int apfs_table_locate_data(struct apfs_table *table,
				  int index, int *off);
extern int apfs_table_query(struct super_block *sb, struct apfs_query *query);

void apfs_table_get(struct apfs_table *table);
void apfs_table_put(struct apfs_table *table);
#endif	/* _APFS_TABLE_H */
