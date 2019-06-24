#include "annot.h"

struct Obj {
  int data;
};

struct Log {
  log_field int data;
  log_field Obj obj;

  void log_fnc correct() {
    tx_begin();
    log(&data);
    data = 6;
    tx_end();
  }

  void log_fnc notLogged() {
    tx_begin();
    data = 5;
    tx_end();
  }

  void logData() { log(&data); }

  void log_fnc doubleLogged() {
    tx_begin();
    log(&data);
    logData();
    data = 5;
    tx_end();
  }

  void log_fnc outsideTx() {
    log(&data);
    data = 5;
  }

  void writeObj() { obj.data = 5; }
};

void log_fnc correctWriteObj(Log* l) {
  tx_begin();
  log(l);
  l->writeObj();
  tx_end();
}