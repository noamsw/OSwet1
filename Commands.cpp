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

void cleanUp(int num_args, char** arguments)
{
    for (int i = 0 ; i < num_args ; i++)
    {
        free(arguments[i]);
        arguments[i] = nullptr;
    }
}

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


Command::Command(const char* cmd_line) : job_id(-1), p_id(-1), cmd_line(cmd_line)
{
    num_args = _parseCommandLine(cmd_line, arguments);
}

Command::~Command()
{
    cleanUp(num_args, arguments);
}

BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line){}

ExternalCommand::ExternalCommand(const char* cmd_line) : Command(cmd_line)
{
    char new_line[COMMAND_ARGS_MAX_LENGTH];
    strcpy(new_line, cmd_line);
    if (_isBackgroundComamnd(cmd_line))
    {
        _removeBackgroundSign(new_line);
    }
    num_args_no_bg = _parseCommandLine(new_line, arguments_no_bg);
}

ExternalCommand::~ExternalCommand()
{
    cleanUp(num_args_no_bg, arguments_no_bg);
}

void ExternalCommand::execute()
{
    pid_t returned_pid = fork();
    if (returned_pid == 0) // son
    {
        execv("\bin\bash", arguments_no_bg);
    }
    else // father
    {
        if(_isBackgroundComamnd(cmd_line))
        {
            SmallShell::getInstance().jobslist.addJob(this , false);
        }
        else // its fg
        {
            int wstaus;
            waitpid(returned_pid, &wstaus, 0); // check if options should be 0
        }
    }
}

int SmallShell::get_a_job_id() {
    // will returnt the current id open for a job and increment
    int  id = SmallShell::getInstance().max_job_id;
    SmallShell::getInstance().max_job_id++;
    return id;
}

void JobsList::addJob(Command* cmd, bool isStopped) {
    if(cmd->job_id == -1)
    {
        cmd->job_id = SmallShell::getInstance().get_a_job_id();
    }
    time_t time_entered;
    time(&time_entered);
    JobEntry newjob(cmd->job_id, cmd->p_id, time_entered, cmd->cmd_line, isStopped);

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
    for(auto  it = jobslist.begin(); it != jobslist.end(); ++it) {// a little confused with variable type
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
            cout << "[" << it.job_id << "]" << it.cmd << ":" << it.cmd_pid << seconds_since << " secs (stopped)" << endl;
        else   cout << "[" << it.job_id << "]" << it.cmd << ":" << it.cmd_pid << seconds_since << " secs" << endl;
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) { //returns nullptr if no job exists
    JobEntry* job = nullptr;
    for (auto & it  : jobslist) {
        if(it.job_id == jobId)
            job = &it; // i think that this will give me the address of the job stored in the vector
    }
    return job;
}

/*
void JobsList::removeJobById(int jobId)
{
    it
    jobslist.erase(it);
}
*/

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId)
{
    JobEntry* job = nullptr;
    for (auto & it  : jobslist) {
        if(it.isStopped)
        {
            job = &it;
            *jobId = it.job_id;
        }
    }
    return job;
}

void JobsList::killAllJobs()
{
    for (auto & it  : jobslist)
    {
        cout << it.cmd_pid << ": " << it.cmd << endl;
        kill(it.cmd_pid, SIGKILL);
    }

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
      cleanUp(num_args, arguments);
  }

  else if (firstWord.compare("showpid") == 0)
  {
      cout << "smash pid is " << getpid() << endl;
  }

  else if (firstWord.compare("pwd") == 0) {
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
            cleanUp(num_args, arguments);
            return nullptr;
        }

        if (new_path.compare("-") == 0)
        {
            if (prev_dir == nullptr)
            {
                cout << "smash error: cd: OLDPWD not set" << endl;
                cleanUp(num_args, arguments);
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
        cleanUp(num_args, arguments);
        return nullptr;
  }

  else if(firstWord.compare("jobs") == 0){
      jobslist.printJobsList();
  }

  else if(firstWord.compare("kill") == 0){
        char* arguments[COMMAND_MAX_ARGS];
        int num_args = _parseCommandLine(new_line, arguments);
        string sig = arguments[1];
        string id = arguments[2];
        string sig_num = arguments[1];
        sig_num = sig_num.substr(1);
        bool sig_not_digits = true;
        bool id_not_digits = true;
        if(sig_num.find_first_not_of("1234567890") == std::string::npos)// check to see that a number was entered
            sig_not_digits = false; //im not 100% sure that this is what i need to check, perhaps by val of signals
        if(id.find_first_not_of("1234567890") == std::string::npos)// check to see that a number was entered
            id_not_digits = false;
        if(num_args != 3 || sig[0] != '-' || sig_not_digits || id_not_digits){
            cout << "smash error: kill: invalid arguments" << endl;
            cleanUp(num_args, arguments);
            return nullptr;
        }
        int id_int = atoi(arguments[2]);
        JobsList::JobEntry* job = jobslist.getJobById(id_int);
        if(!job){
            cout << "smash error: kill: job-id" << id << "does not exist" << endl;
            cleanUp(num_args, arguments);
            return nullptr;
        }
        stringstream st_sig(sig_num);
        int sig_int = 0;
        st_sig >> sig_int;
        int return_val =  kill(job->cmd_pid, sig_int);// do we need to check that this is not sig_cont
        if (return_val){
            perror("smash error: kill failed");
        }
        else
            cout << "signal number" << sig_num << "was sent to pid" << job->cmd_pid << endl;
        cleanUp(num_args, arguments);
  }

  else if (firstWord.compare("fg") == 0)
  {
      char* arguments[COMMAND_MAX_ARGS];
      int num_args = _parseCommandLine(new_line, arguments);

      // check if the syntax (num_args and format of the command) is valid
      // whats the meaning of "format"? make sure its a number?
      // not_digit
      if (num_args > 2)
      {
          cout << "smash error: fg: invalid arguments" << endl;
          cleanUp(num_args, arguments);
          return nullptr;
      }

      // find the command in the jobs list:
      int job_id;
      if (num_args == 1) // job_id isnt given
          job_id = max_job_id - 1; // need to fix
      else // job_id is given
        job_id = atoi(arguments[1]);

      JobsList::JobEntry* cur_job = jobslist.getJobById(job_id);
      if (num_args >= 2 && cur_job == nullptr)
      {
          cout << "smash error: fg: job-id " << job_id << " does not exist" << endl;
          cleanUp(num_args, arguments);
          return nullptr;
      }
      if (num_args == 1 && cur_job == nullptr)
      {
          cout << "smash error: fg: jobs list is empty" << endl;
          cleanUp(num_args, arguments);
          return nullptr;
      }

      cout << cur_job->cmd << " : " << cur_job->cmd_pid << endl;

      if (cur_job->isStopped)
        kill(cur_job->cmd_pid ,SIGCONT);

      int wstaus;
      waitpid(cur_job->cmd_pid, &wstaus, 0); // is 0 the right value for the "options" arg?

      // jobslist.removeJobById(job_id); // not implemented yet

      cleanUp(num_args, arguments);
      return nullptr;
  }

  else if (firstWord.compare("bg") == 0)
  {
      char* arguments[COMMAND_MAX_ARGS];
      int num_args = _parseCommandLine(new_line, arguments);

      // check if the syntax (num_args and format of the command) is valid
      // whats the meaning of "format"? make sure its a number?
      if (num_args > 2)
      {
          cout << "smash error: bg: invalid arguments" << endl;
          cleanUp(num_args, arguments);
          return nullptr;
      }

      // find the command in the jobs list:
      JobsList::JobEntry* cur_job;
      if (num_args == 1) // job_id isnt given
      {
          int job_id;
          cur_job = jobslist.getLastStoppedJob(&job_id);
          if (cur_job == nullptr)
          {
              cout << "smash error: bg: there is no stopped jobs to resume" << endl;
              cleanUp(num_args, arguments);
              return nullptr;
          }
      }
      else // job_id is given
      {
          int job_id = atoi(arguments[1]);
          cur_job = jobslist.getJobById(job_id);
          if (cur_job == nullptr)
          {
              cout << "smash error: bg: job-id " << job_id << " does not exist" << endl;
              cleanUp(num_args, arguments);
              return nullptr;
          }
          if (!cur_job->isStopped)
          {
              cout << "smash error: bg: job-id " << job_id << " is already running in the background" << endl;
              cleanUp(num_args, arguments);
              return nullptr;
          }
      }

      cout << cur_job->cmd << " : " << cur_job->cmd_pid << endl;
      cur_job->isStopped = false;
      kill(cur_job->cmd_pid, SIGCONT);
      cleanUp(num_args, arguments);
      return nullptr;
  }

  else if (firstWord.compare("quit") == 0)
  {
      char* arguments[COMMAND_MAX_ARGS];
      int num_args = _parseCommandLine(new_line, arguments);

      if (num_args >= 2)
      {
          if (arguments[1] == "kill")
          {
              cout << "smash: sending SIGKILL signal to " << jobslist.jobslist.size() << " jobs:" << endl;
              jobslist.killAllJobs();
          }
      }
      cleanUp(num_args, arguments);
      exit (0);
  }

  // else if special commands?
  else
  {
        return new ExternalCommand(cmd_line); // check if we need to free it later or not (AND HOW?)
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    Command* cmd = CreateCommand(cmd_line);
    if (cmd != nullptr)
  {
      cmd->execute();
      delete(cmd);
  }
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}