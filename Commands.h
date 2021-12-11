#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include <string.h>
#include <vector>
#include <map>
#include <linux/limits.h>
#include <fcntl.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members
 public:
  pid_t p_id;
  int num_args;
  char* arguments[COMMAND_MAX_ARGS];
  int job_id; //i think we will need to store the job id in the cmd inorder to save it if it has been added than removed from the jobsList
  const char* cmd_line;

  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  // virtual void prepare();
  // virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  char* bash_args[4];
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand(){};
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  char first_command[COMMAND_ARGS_MAX_LENGTH];
  char second_command[COMMAND_ARGS_MAX_LENGTH];
  bool err_pipe; // true if the command is |&
  PipeCommand(const char* cmd_line, char* first_part, char* second_part, bool err_pipe_) :
          Command(cmd_line), err_pipe(err_pipe_)
  {
      strcpy(first_command, first_part);
      strcpy(second_command, second_part);
  }
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  char cmd_line_no_rd[COMMAND_ARGS_MAX_LENGTH];
  char file_name[COMMAND_ARGS_MAX_LENGTH];
  bool append; // true if the command is (>>)
  explicit RedirectionCommand(const char* cmd_line, char* c_l_n_rd, char* f_n, bool append) :
        Command(cmd_line), append(append)
  {
      strcpy(cmd_line_no_rd, c_l_n_rd);
      strcpy(file_name, f_n);
  }
  virtual ~RedirectionCommand() {}
  void execute() override;
  // void prepare() override;

  // void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};
/*
class GetCurrDirCommand : public BuiltInCommand {
 public:
//  char* pwd = nullptr; not sure we need this
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand(){}
  void execute() override;
};
*/
class JobsList {
public:
    class JobEntry {//deleted the job class and instantiated the job entry class that they gave us, seemed right
    public:
        int job_id;//job id assigned by smash
        pid_t cmd_pid;// process pid assigned after the fork
        time_t t_entered;// time when inserted from the epoch in 1970.
        char cmd_line[COMMAND_ARGS_MAX_LENGTH];// the cmd of the job
        bool isStopped;//was the job stopped or simply sent to the bg
        JobEntry(int job_id, pid_t cmd_pid, time_t t_entered, const char* cmd, bool isStopped) : job_id(job_id), cmd_pid(cmd_pid), t_entered(t_entered), isStopped(isStopped) {
            strcpy(cmd_line, cmd);
        }
    };
    std::vector<JobEntry> jobslist;
public:
    JobsList() = default; // i think that there isnt anything special here
    ~JobsList() = default; // same as above
    void addJob(Command* cmd, bool isStopped = false); // a method for adding jobs to the job list
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId); // NEED TO IMPLEMENT
    // JobEntry * getLastJob(int* lastJobId); // I think we dont need this
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

/*
class PWDCommand : public BuiltInCommand {
 public:
  char* pwd = nullptr;
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};
*/
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class HeadCommand : public BuiltInCommand {
 public:
  HeadCommand(const char* cmd_line);
  virtual ~HeadCommand() {}
  void execute() override;
};

class SmallShell {
 private:
  // TODO: Add your data members
  //SmallShell();
 public:
  int max_job_id;
  char* prev_dir;
  std::string prompt;
  bool p_running; //flag to see if there is a process running inside the shell
  Command *cur_cmd;
  JobsList jobslist; //switched from std::vector for BG and stopped jobs
protected: //how to make sure that only a singleton is created
  SmallShell(const std::string& prompt = "smash> ") : max_job_id(1), prev_dir(nullptr), prompt(prompt),p_running(false), cur_cmd(nullptr) {} // check if ok i initialized the cur_pid to -1 but it is arbitrary i think
public:
  Command *CreateCommand(const char* cmd_line);
  int get_a_job_id();  // will return an id that should be assigned to a new job
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable assignment operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
};


#endif //SMASH_COMMAND_H_
//