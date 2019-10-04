#include "annot.h"
#include "nvml/include/libpmemobj.h"

#undef TX_BEGIN
#undef TX_ONABORT
#undef TX_END
#undef TX_ADD
#undef TX_ADD_FIELD

#define TX_BEGIN(pop) tx_begin();
#define TX_ONABORT
#define TX_END tx_end();
#define TX_ADD(node) tx_log(D_RW(node))
#define TX_ADD_FIELD(node, field) tx_log(&D_RW(node)->field)

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

void nvm_fnc correct(TOID(struct tree_map_node) node) {
  TX_BEGIN(pop)
  TX_ADD(node);
  D_RW(node)->a = 0;
  D_RW(node)->b = 1;
  D_RW(node)->c = TX_ZNEW(struct tree_map_node);
  TX_END
}

void nvm_fnc commitBug(TOID(struct tree_map_node) node) {
  TX_BEGIN(pop)
  if (cond()) {
     TX_ADD(node);
  }
 
  D_RW(node)->a = 0;
  TX_END
}

void nvm_fnc doubleLogBug(TOID(struct tree_map_node) node) {
  TX_BEGIN(pop)
  TX_ADD(node);
  D_RW(node)->a = 0;

  if (cond()) {
    D_RW(node)->b = 1;
    TX_ADD_FIELD(node, c);
  }

  D_RW(node)->c = TX_ZNEW(struct tree_map_node);
  TX_END
}

void nvm_fnc outTxBug(TOID(struct tree_map_node) node) {
  D_RW(node)->a = 0;
  TX_END
}