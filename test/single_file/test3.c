#include "annot.h"
#include "nvml/include/libpmemobj.h"
#include "annot_nvml.h"

#define BTREE_MAP_TYPE_OFFSET 1012
struct btree_map;
TOID_DECLARE(struct btree_map, BTREE_MAP_TYPE_OFFSET + 0);
TOID_DECLARE(struct tree_map_node, BTREE_MAP_TYPE_OFFSET + 1);
#define BTREE_ORDER 8
#define BTREE_MIN ((BTREE_ORDER / 2) - 1)
#define EMPTY_ITEM ((struct tree_map_node_item){0, OID_NULL})

struct tree_map_node {
  int a;
  int b;
  TOID(struct tree_map_node) c;
};

struct btree_map {
	TOID(struct tree_map_node) root;
};

void ipafnc(TOID(struct tree_map_node) node3) {
  TOID(struct tree_map_node) node4 = TX_ZNEW(struct tree_map_node);
  D_RW(node3)->a = 5;
  D_RW(node3)->b = 6;
  D_RW(node4)->a = 4;
  D_RW(node4)->c = node3;
}

int tx_fnc main() {
  TX_BEGIN(x)
  TOID(struct tree_map_node) node = TX_ZNEW(struct tree_map_node);
  TOID(struct tree_map_node) node2 = TX_ZNEW(struct tree_map_node);
  ipafnc(node);
  D_RW(node)->a = 2;
  D_RW(node)->b = 3;
  D_RW(node2)->a = 1;
  D_RW(node2)->b = 9;
  TX_END
}
