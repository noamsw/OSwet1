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
    if(SmallShell::getInstance().p_running){
        int return_val =  kill(SmallShell::getInstance().cur_pid, 9);
        SmallShell::getInstance().p_running = false;
        if (return_val){
            std::cout << "houston we have a problem" << endl; //this is just a check if killing a proccess doesnt work will remove before final issue.
        }
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

//