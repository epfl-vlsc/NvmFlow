#include "annot.h"
#include <cstdlib>
#include <cstring>

#define PM_EQU(pm_dst, y) pm_dst = y;
#define PM_OR_EQU(pm_dst, y) pm_dst |= y;
#define PM_MEMCPY(pm_dst, src, sz) memcpy(pm_dst, src, sz);

extern void pmfs_flush_buffer(void* buf, int len, bool fence);

struct pmfs_sb_info {
  void* s_bdev;
  void* phys_addr;
  void* virt_addr;
  unsigned long block_start;
  unsigned long block_end;
  unsigned long num_free_blocks;
  int next_transaction_id;
  int jsize;
  void* journal_base_addr;
  bool redo_log;
  unsigned long bpi;
  unsigned long num_inodes;
  unsigned long blocksize;
  unsigned long initsize;
  unsigned long s_mount_opt;
};

struct super_block {
  pmfs_sb_info* s_fs_info;
};

typedef struct pmfs_journal {
  int base;
  int size;
  int head;
  int tail;
  int gen_id;
  int pad;
  int redo_logging;
} pmfs_journal_t;

typedef struct {
  int addr_offset;
  int transaction_id;
  int gen_id;
  int type;
  int size;
  char data[48];
} pmfs_logentry_t;

typedef struct pmfs_transaction {
  int transaction_id;
  int num_entries;
  int num_used;
  int gen_id;
  int status;
  pmfs_journal_t* t_journal;
  sentinel() pmfs_logentry_t* start_addr;
  struct pmfs_transaction* parent;
} pmfs_transaction_t;

static inline struct pmfs_sb_info* PMFS_SB(super_block* sb) {
  return sb->s_fs_info;
}

static inline struct pmfs_super_block* pmfs_get_super(super_block* sb) {
  struct pmfs_sb_info* sbi = PMFS_SB(sb);

  return (struct pmfs_super_block*)sbi->virt_addr;
}

static inline int pmfs_get_addr_off(pmfs_sb_info* sbi, void* addr) {
  return (int)((char*)addr - (char*)sbi->virt_addr);
}

static inline void* pmfs_get_block(super_block* sb, int block) {
  struct pmfs_super_block* ps = pmfs_get_super(sb);

  return block ? ((char*)ps + block) : NULL;
}

static void pmfs_flush_transaction(struct super_block* sb,
                                   pmfs_transaction_t* trans) {
  struct pmfs_sb_info* sbi = PMFS_SB(sb);
  pmfs_logentry_t* le = trans->start_addr;
  int i;
  char* data;

  for (i = 0; i < trans->num_used; i++, le++) {
    if (le->size) {
      data = (char*)pmfs_get_block(sb, le->addr_offset);
      if (sbi->redo_log) {
        PM_MEMCPY(data, le->data, le->size);
      } else
        pmfs_flush_buffer(data, le->size, false);
    }
  }
}

static inline void pmfs_commit_logentry(struct super_block* sb,
                                        pmfs_transaction_t* trans,
                                        pmfs_logentry_t* le) {
  struct pmfs_sb_info* sbi = PMFS_SB(sb);
  if (sbi->redo_log) {
    pfence();
    PM_OR_EQU(le->type, 1);
    PM_EQU(le->gen_id, trans->gen_id);
    pmfs_flush_buffer(le, sizeof(pmfs_logentry_t), false);
    pfence();
    pmfs_flush_transaction(sb, trans);
  } else {
    pmfs_flush_transaction(sb, trans);
    pfence();
    PM_OR_EQU(le->type, 1);
    PM_EQU(le->gen_id, trans->gen_id);
    pmfs_flush_buffer(le, sizeof(pmfs_logentry_t), true);
  }
}

int nvm_fnc pmfs_add_logentry(struct super_block* sb, pmfs_transaction_t* trans,
                              void* addr, int size, int type) {
  struct pmfs_sb_info* sbi = PMFS_SB(sb);
  pmfs_logentry_t* le;
  int num_les = 0, i;
  int le_start = size ? pmfs_get_addr_off(sbi, addr) : 0;
  int le_size;

  le = trans->start_addr + trans->num_used;

  for (i = 0; i < num_les; i++) {
    PM_EQU(le->addr_offset, le_start);
    PM_EQU(le->transaction_id, trans->transaction_id);
    le_size = (i == (num_les - 1)) ? size : sizeof(le->data);
    PM_EQU(le->size, le_size);
    size -= le_size;
    if (le_size)
      PM_MEMCPY(le->data, addr, le_size);
    PM_EQU(le->type, type);
    if (i == 0 && trans->num_used == 0)
      PM_OR_EQU(le->type, 1);
    trans->num_used++;
    if (i == (num_les - 1) && (type & 1)) {
      pmfs_commit_logentry(sb, trans, le);
      return 0;
    }
    pfence();
    PM_EQU(le->gen_id, trans->gen_id);
    pmfs_flush_buffer(le, sizeof(le), false);
    addr = (void*)((char*)addr + le_size);
    le_start += le_size;
    le++;
  }

  if (!sbi->redo_log) {
    pfence();
  }
  return 0;
}
