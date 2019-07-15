#include "annot.h"

struct Log {
  int data;
  log_field int valid;

  void log_fnc outsideTx() {
    log(&valid);
    valid = 5;
  }

  void log_fnc correct() {
    tx_begin();
    log(&valid);
    valid = 6;
    tx_end();
  }

  void log_fnc correct2() {
    tx_begin();
    log(&valid);
    if (valid == 5)
      valid = 6;
    tx_end();
  }

  void log_fnc notLogged() {
    tx_begin();
    valid = 5;
    tx_end();
  }

  void logValid() { log(&valid); }

  void log_fnc doubleLogged() {
    data = 5;
    tx_begin();
    log(&valid);
    logValid();
    valid = 5;
    tx_end();
  }

  void log_fnc correctObj() {
    tx_begin();
    log(this);
    valid = 5;
    tx_end();
  }

  void log_fnc missLog() {
    tx_begin();
    if (valid == 6) {
      log(&valid);
    }
    valid = 1;
    tx_end();
  }

  void log_fnc loopLog() {
    tx_begin();
    while (true) {
      log(this);
      valid = 1;
    }
    tx_end();
  }

  void condLog() {
    if (valid == 6) {
      log(this);
    }
  }

  void log_fnc ipaMissLog() {
    tx_begin();
    condLog();
    valid = 1;
    tx_end();
  }

  void log_fnc recurse() {
    tx_begin();
    log(&valid);
    recurse();
    tx_end();
  }
};