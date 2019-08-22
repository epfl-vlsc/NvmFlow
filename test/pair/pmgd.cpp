#include "annot.h"
#include <cstdlib>
#include <cstring>

namespace PMGD {
class TransactionImpl {
  struct JournalEntry;
  sentinel(TransactionImpl::JournalEntry) JournalEntry* _jcur;

public:
  void log_je(void* src, int len);
  void je_flush(JournalEntry* addr);
};
} // namespace PMGD

using namespace PMGD;
struct TransactionImpl::JournalEntry {
  long tx_id : 56;
  long len : 8;
  void* addr;
  int data[50];
};

void nvm_fnc TransactionImpl::log_je(void* src_ptr, int len) {
  _jcur->len = len;
  _jcur->addr = src_ptr;
  memcpy(&_jcur->data[0], src_ptr, len);
  _jcur->tx_id = 1;
  je_flush(_jcur);
  _jcur++;
}
