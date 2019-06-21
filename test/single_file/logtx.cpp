#include "annot.h"

struct Log {
  log_field int data;

  void analyze_logging correct() {
    tx_begin();
    log(&data);
    data = 6;
    tx_end();
  }

  void analyze_logging notLogged() {
    tx_begin();
    data = 5;
    tx_end();
  }

  void logData(){
    log(&data);
  }

  void analyze_logging doubleLogged() {
    tx_begin();
    log(&data);
    logData();
    data = 5;
    tx_end();
  }

  void analyze_logging outsideTx() {
    log(&data);
    data = 5;
  }
};
