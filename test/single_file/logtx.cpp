#include "annot.h"
#include "nvml/include/libpmemobj.h"

#define BTREE_MAP_TYPE_OFFSET 1012
struct btree_map;
TOID_DECLARE(struct btree_map, BTREE_MAP_TYPE_OFFSET + 0);
TOID_DECLARE(struct tree_map_node, BTREE_MAP_TYPE_OFFSET + 1);
#define BTREE_ORDER 8
#define BTREE_MIN ((BTREE_ORDER / 2) - 1)

struct tree_map_node_item {
  uint64_t key;
  PMEMoid value;
};

#define EMPTY_ITEM ((struct tree_map_node_item){0, OID_NULL})

struct tree_map_node {
  int n; /* number of occupied slots */
  struct tree_map_node_item items[BTREE_ORDER - 1];
  TOID(struct tree_map_node) slots[BTREE_ORDER];
};

struct btree_map {
  TOID(struct tree_map_node) root;
};

int btree_map_new(PMEMobjpool* pop, TOID(struct btree_map) * map, void* arg) {
  int ret = 0;

  TX_BEGIN(pop) {
    pmemobj_tx_add_range_direct(map, sizeof(*map));
    *map = TX_ZNEW(struct btree_map);
  }
  TX_ONABORT { ret = 1; }
  TX_END

  return ret;
}