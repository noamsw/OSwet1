#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>

using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    if(SmallShell::getInstance().p_running){ // check if a process is running, if so
        // cout << "were here" << endl;
        pid_t pid_to_kill = SmallShell::getInstance().cur_cmd->p_id; //get the pid to stop and add to jobs list maybe check this? not nullptr?
        SmallShell::getInstance().jobslist.addJob(SmallShell::getInstance().cur_cmd, true);// add it to the jobs list
        int return_val =  kill(pid_to_kill, 19);
        if (return_val){
            perror("smash error: kill failed");
        }
        cout << "smash: process " << pid_to_kill <<  " was stopped" << endl;
        SmallShell::getInstance().p_running = false;
        SmallShell::getInstance().cur_cmd = nullptr;
    }
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    if(SmallShell::getInstance().p_running){
        pid_t pid_to_kill = SmallShell::getInstance().cur_cmd->p_id;
        int return_val =  kill(pid_to_kill, 9);
        if (return_val){
            perror("smash error: kill failed");
        }
        cout << "smash: process " << pid_to_kill <<  " was killed" << endl;
        SmallShell::getInstance().p_running = false;
        SmallShell::getInstance().cur_cmd = nullptr;
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

//