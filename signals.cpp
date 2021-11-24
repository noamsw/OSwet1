#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    // recive 20, send 19
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
  // send 9 to process in FG (child of smash)



}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

//