#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <sys/types.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

const string WHITESPACE = " \n\r\t\f\v";

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 


Command::Command(const char* cmd_line, pid_t p_id) : job_id(-1), p_id(p_id), cmd_line(cmd_line)
{
    num_args = _parseCommandLine(cmd_line, arguments);
}

Command::~Command()
{
    for (int i = 0 ; i < num_args ; i++)
    {
        free(arguments[i]);
    }
}

BuiltInCommand::BuiltInCommand(const char* cmd_line, pid_t p_id) : Command(cmd_line, p_id){}
/*
GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}
void GetCurrDirCommand::execute() {
    char* cwd = nullptr;
    getcwd(cwd, 0);
    if(cwd){
        std::string pwd(cwd);
        std::cout << pwd << endl;
        free(cwd);
        return;
    }
    cout<< "cwd failed" <<endl;
}
*/
//SmallShell::SmallShell() {
// TODO: add your implementation
//}

int SmallShell::get_a_job_id() {
    // will returnt the current id open for a job and increment
    int  id = SmallShell::getInstance().max_job_id;
    SmallShell::getInstance().max_job_id++;
    return id;
}
void JobsList::addJob(Command* cmd, bool isStopped) {
    time_t time_entered;
    time(&time_entered);
    JobEntry newjob(cmd->job_id, cmd->p_id, time_entered, cmd->cmd_line, isStopped);
    if(cmd->job_id == -1)
        newjob.job_id = SmallShell::getInstance().get_a_job_id();
    removeFinishedJobs();
    bool inserted = false;
    for(std::vector<JobEntry>::iterator it = jobslist.begin(); it != jobslist.end(); ++it){
        if(it->job_id > newjob.job_id){
            jobslist.insert(it, newjob);
            inserted = true;
        }
    }
    if(!inserted)
        jobslist.insert(jobslist.end(), newjob);
}

void JobsList::removeFinishedJobs() {
    int status;
    for(auto  it = jobslist.begin(); it != jobslist.end(); ++it) {
        waitpid(it->job_id, &status, WNOHANG);
        if (status != 0) { //im ignoring status -1 which means there was an error and assuming that it was dealt with
            it = jobslist.erase(it);
        }
    }
}

void JobsList::printJobsList() {
    removeFinishedJobs();
    time_t time_now;
    time(&time_now);
    for(auto & it : jobslist){
        time(&time_now); // im not sure how much of a diff itll make but i updated the time in the loop
        double seconds_since = difftime(it.t_entered, time_now);
        if(it.isStopped)
            cout << "[" << it.job_id << "]" << it.cmd << ":" << it.cmd_pid << seconds_since << " secs (stopped)";
        else   cout << "[" << it.job_id << "]" << it.cmd << ":" << it.cmd_pid << seconds_since << " secs";
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    JobEntry* job = nullptr;
    for (auto & it  : jobslist) {
        if(it.job_id == jobId)
            job = &it; // i think that this will give me the address of the job stored in the vector
    }
    return job;
}

SmallShell::~SmallShell()
{
    if(prev_dir){
        free(prev_dir);
        prev_dir = nullptr;
    }
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    char new_line[COMMAND_ARGS_MAX_LENGTH];
    strcpy(new_line, cmd_line);

  // if the command is builtin it cant be BG
  // remove "&' if exists
  if (firstWord == "chprompt" || firstWord == "showpid" || firstWord == "pwd" || firstWord == "cd" ||
          firstWord == "jobs" || firstWord == "kill" || firstWord == "fg" || firstWord == "bg" || firstWord == "quit")
  {
      if (_isBackgroundComamnd(cmd_line))
      {
          _removeBackgroundSign(new_line);
      }
  }

  if (firstWord.compare("chprompt") == 0)
  {
      char* arguments[COMMAND_MAX_ARGS];
      int num_args = _parseCommandLine(new_line, arguments);
      if (num_args == 1)
      {
          prompt = "smash> ";
      } else
      {
          prompt = arguments[1];
          prompt.append("> ");
      }
    for (int i=0 ; i<num_args ; i++)
    {
        free(arguments[i]);
    }
  }
///
  else if (firstWord.compare("showpid") == 0)
  {
      cout << "smash pid is " << getpid() << endl;
  }

  else if (firstWord.compare("pwd") == 0) {
        //return new GetCurrDirCommand(cmd_line);
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
        std::string pwd(cwd);
        std::cout << pwd << endl;
    }

  else if (firstWord.compare("cd") == 0){
        char* arguments[COMMAND_MAX_ARGS];
        int num_args = _parseCommandLine(new_line, arguments);
        string new_path = arguments[1];

        if (num_args > 2)
        {
            cout << "smash error: cd: too many arguments" << endl;
            for (int i=0 ; i<num_args ; i++)
            {
                free(arguments[i]);
            }
            return nullptr;
        }

        if (new_path.compare("-") == 0)
        {
            if (prev_dir == nullptr)
            {
                cout << "smash error: cd: OLDPWD not set" << endl;
                for (int i=0 ; i<num_args ; i++)
                {
                    free(arguments[i]);
                }
                return nullptr;
            }
            char tmp_path[PATH_MAX];
            strcpy(tmp_path, prev_dir);
            free(prev_dir);
            prev_dir = nullptr;
            char cwd[PATH_MAX];
            getcwd(cwd, PATH_MAX);
            prev_dir = (char*)(malloc(PATH_MAX));
            strcpy(prev_dir, cwd);
            chdir(tmp_path);
        }
        else
        {
            if(prev_dir) {
                free(prev_dir);
                prev_dir = NULL;
            }
            char cwd[PATH_MAX];
            getcwd(cwd, PATH_MAX);
            prev_dir = (char*)(malloc(PATH_MAX));
            strcpy(prev_dir, cwd);
            chdir(arguments[1]);
        }
        for (int i=0 ; i<num_args ; i++)
        {
            free(arguments[i]);
        }
        return nullptr;
  }
  else if(firstWord.compare("jobs") == 0){
      jobslist.printJobsList();
  }
  else if(firstWord.compare("kill")){
        char* arguments[COMMAND_MAX_ARGS];
        int num_args = _parseCommandLine(new_line, arguments);
        string sig = arguments[1];
        string id = arguments[2];
        string sig_num = arguments[1];
        sig_num = sig_num.substr(1);
        bool not_digits = true;
        if(sig_num.find_first_not_of("1234567890") == std::string::npos)// check to see that a number was entered
            not_digits = false; //im not 100% sure that this is what i need to check, perhaps by val of signals
        if(id.find_first_not_of("1234567890") == std::string::npos)// check to see that a number was entered
            not_digits = false;
        if(num_args != 3 || sig[0] != '-' || not_digits){
            cout << "smash error: kill: invalid arguments" << endl;
            return nullptr;
        }
        stringstream st(id);
        int id_int = 0;
        st >> id_int;
        JobsList::JobEntry* job = jobslist.getJobById(id_int);
        if(!job){
            cout << "smash error: kill: job-id" << id << "does not exist" << endl;
            return nullptr;
        }
        stringstream st_sig(sig_num);
        int sig_int = 0;
        st_sig >> sig_int;
        int return_val =  kill(job->cmd_pid, sig_int);
        if (return_val){
            perror("smash error: kill failed"); //this is just a check if killing a proccess doesnt work will remove before final issue.
        }
        else
            cout << "signal number" << sig_num << "was sent to pid" << job->cmd_pid << endl;
  }
    /*
    else if ...
    .....
    else {
      return new ExternalCommand(cmd_line);
    }
    */
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  Command* cmd = CreateCommand(cmd_line);
  if (cmd != nullptr)
  {
      cmd->execute();
  }
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}//