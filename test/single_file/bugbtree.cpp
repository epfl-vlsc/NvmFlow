#include "annot.h"
#include "nvml/include/libpmemobj.h"

#undef TX_BEGIN
#undef TX_ONABORT
#undef TX_END

#define TX_BEGIN(pop) tx_begin(pop);
#define TX_ONABORT
#define TX_END tx_end();

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
  int n;
  struct tree_map_node_item items[BTREE_ORDER - 1];
  TOID(struct tree_map_node) slots[BTREE_ORDER];
};

struct btree_map {
  TOID(struct tree_map_node) root;
};

int btree_map_is_empty(PMEMobjpool* pop, TOID(struct btree_map) map) {
  return TOID_IS_NULL(D_RO(map)->root) || D_RO(D_RO(map)->root)->n == 0;
}

static void btree_map_insert_item_at(TOID(struct tree_map_node) node, int pos,
                                     struct tree_map_node_item item) {
  D_RW(node)->items[pos] = item;
  D_RW(node)->n += 1;
}

static TOID(struct tree_map_node)
    btree_map_create_split_node(TOID(struct tree_map_node) node,
                                struct tree_map_node_item* m) {
  TOID(struct tree_map_node) right = TX_ZNEW(struct tree_map_node);

  int c = (BTREE_ORDER / 2);
  *m = D_RO(node)->items[c - 1]; /* select median item */
  D_RW(node)->items[c - 1] = EMPTY_ITEM;

  /* move everything right side of median to the new node */
  for (int i = c; i < BTREE_ORDER; ++i) {
    if (i != BTREE_ORDER - 1) {
      D_RW(right)->items[D_RW(right)->n++] = D_RO(node)->items[i];

      D_RW(node)->items[i] = EMPTY_ITEM;
    }
    D_RW(right)->slots[i - c] = D_RO(node)->slots[i];
    D_RW(node)->slots[i] = TOID_NULL(struct tree_map_node);
  }
  D_RW(node)->n = c - 1;

  return right;
}

static void btree_map_insert_node(TOID(struct tree_map_node) node, int p,
                                  struct tree_map_node_item item,
                                  TOID(struct tree_map_node) left,
                                  TOID(struct tree_map_node) right) {
  TX_ADD(node);
  if (D_RO(node)->items[p].key != 0) { /* move all existing data */
    memmove(&D_RW(node)->items[p + 1], &D_RW(node)->items[p],
            sizeof(struct tree_map_node_item) * ((BTREE_ORDER - 2 - p)));

    memmove(&D_RW(node)->slots[p + 1], &D_RW(node)->slots[p],
            sizeof(TOID(struct tree_map_node)) * ((BTREE_ORDER - 1 - p)));
  }
  D_RW(node)->slots[p] = left;
  D_RW(node)->slots[p + 1] = right;
  btree_map_insert_item_at(node, p, item);
}

static TOID(struct tree_map_node)
    btree_map_find_dest_node(TOID(struct btree_map) map,
                             TOID(struct tree_map_node) n,
                             TOID(struct tree_map_node) parent, uint64_t key,
                             int* p) {
  if (D_RO(n)->n == BTREE_ORDER - 1) { /* node is full, perform a split */
    struct tree_map_node_item m;
    TOID(struct tree_map_node) right = btree_map_create_split_node(n, &m);

    if (!TOID_IS_NULL(parent)) {
      btree_map_insert_node(parent, *p, m, n, right);
      if (key > m.key) /* select node to continue search */
        n = right;
    } else { /* replacing root node, the tree grows in height */
      TOID(struct tree_map_node) up = TX_ZNEW(struct tree_map_node);
      D_RW(up)->n = 1;
      D_RW(up)->items[0] = m;
      D_RW(up)->slots[0] = n;
      D_RW(up)->slots[1] = right;

      TX_ADD_FIELD(map, root);
      D_RW(map)->root = up;
      n = up;
    }
  }

  int i;
  for (i = 0; i < BTREE_ORDER - 1; ++i) {
    *p = i;

    /*
     * The key either fits somewhere in the middle or at the
     * right edge of the node.
     */
    if (D_RO(n)->n == i || D_RO(n)->items[i].key > key) {
      return TOID_IS_NULL(D_RO(n)->slots[i])
                 ? n
                 : btree_map_find_dest_node(map, D_RO(n)->slots[i], n, key, p);
    }
  }

  /*
   * The key is bigger than the last node element, go one level deeper
   * in the rightmost child.
   */
  return btree_map_find_dest_node(map, D_RO(n)->slots[i], n, key, p);
}

static void btree_map_insert_empty(TOID(struct btree_map) map,
                                   struct tree_map_node_item item) {
  TX_ADD_FIELD(map, root);
  D_RW(map)->root = TX_ZNEW(struct tree_map_node);

  btree_map_insert_item_at(D_RO(map)->root, 0, item);
}

static void btree_map_insert_item(TOID(struct tree_map_node) node, int p,
                                  struct tree_map_node_item item) {
  TX_ADD(node);
  if (D_RO(node)->items[p].key != 0) {
    memmove(&D_RW(node)->items[p + 1], &D_RW(node)->items[p],
            sizeof(struct tree_map_node_item) * ((BTREE_ORDER - 2 - p)));
  }
  btree_map_insert_item_at(node, p, item);
}

int btree_map_insert(PMEMobjpool* pop, TOID(struct btree_map) map, uint64_t key,
                     PMEMoid value) {
  struct tree_map_node_item item = {key, value};
  TX_BEGIN(pop) {
    if (btree_map_is_empty(pop, map)) {
      btree_map_insert_empty(map, item);
    } else {
      int p; /* position at the dest node to insert */
      TOID(struct tree_map_node) parent = TOID_NULL(struct tree_map_node);
      TOID(struct tree_map_node)
      dest = btree_map_find_dest_node(map, D_RW(map)->root, parent, key, &p);

      btree_map_insert_item(dest, p, item);
    }
  }
  TX_END

  return 0;
}