#pragma once
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