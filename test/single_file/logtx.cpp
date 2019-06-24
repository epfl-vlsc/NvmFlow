#include "annot.h"

struct Log {
  log_field int data;

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
};
