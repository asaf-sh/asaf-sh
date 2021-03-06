#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define COMMAND_MAX_LENGTH (80)
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define BASH ((char*) "/bin/bash")

using namespace std;
class Command {
// TODO: Add your data members
 private:	  
  void getFgCommandLine(char* fg_command_line);
 protected:
  const char* cmd_line;
  const char* cmd_name;
  char fg_cmd_line[COMMAND_MAX_LENGTH];
 public:
  Command(const char* cmd_line);
  virtual ~Command(){};
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed

  inline const char* getCommandLine() const{
    return cmd_line;
  }
  inline static bool isPipe(const char* cmd_line){
    return string(cmd_line).find('|') != string::npos;
  }

  inline static bool isRedirection(const char* cmd_line){
    return string(cmd_line).find('>') != string::npos;
  }
};

class BuiltInCommand : public Command {
  protected:
  int req_args_len;
  int args_len;
  char* args[COMMAND_MAX_ARGS];
  static int getLen(char** args);
  void setReqArgsLen(int len);
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand(){};
  //static char** parseLine(const char* cmd_line);
  bool virtual validateArgsLen();
  bool virtual validate();
  void virtual prepare();
  void virtual cleanup();
};

class ExternalCommand : public Command {
private:
    char* args[COMMAND_MAX_ARGS+1];
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand(){
	  //printf("in EC dtor\n");
  };
  void execute() override;
  //void inline setPid(pid_t pid) {pid = pid;}
  //pid_t inline getPid() const {return pid;}
};

class PipeCommand : public Command {
 private:
 char cmd_r[COMMAND_MAX_LENGTH];
 Command* cmd_read;
 char cmd_w[COMMAND_MAX_LENGTH];
 Command* cmd_write;
 int write_channel;
 int separatePipe();
 void closePipe(int my_pipe[2]);
 void setWriteSide(int my_pipe[2]);
 void setReadSide(int my_pipe[2]);
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand();
  void execute() override;
};

class RedirectionCommand : public Command {
 private:
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand(){};
  void execute() override;
  bool validateArgs();
  //void prepare() override;
  //void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand{

  public:
  ChangePromptCommand(const char* cmd_line);
  virtual ~ChangePromptCommand() {}
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
  public:
  ChangeDirCommand(const char* cmd_line);
  virtual ~ChangeDirCommand() {}
  void execute() override;
  bool validate() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
  static const int MAX_PATH_LENGTH;
  //bool validate() override;
  static pid_t getPwd();
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
 // bool validate() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
  QuitCommand(const char* cmd_line);
  virtual ~QuitCommand() {}
  void execute() override;
  bool isKill();
};

enum class Status {running, stopped, killed, finished};
class JobsList{
public:
  class JobEntry {
    private:
      char cmd_line[COMMAND_MAX_LENGTH];
      int job_id;
      time_t start_time;
      pid_t pid;
      Status status;
      
      //bool is_fg
    public:
      JobEntry(Command* cmd, int job_id):job_id(job_id){
        //is_fg = !_isBackgroundComamnd(cmd->getCommandLine());
	strcpy(cmd_line, cmd->getCommandLine());
        //_removeBackgroundSign(fg_cmd_line);
      };
      JobEntry(const JobsList::JobEntry &job); 
      ~JobEntry();
      bool isFg();
      inline Status getStatus(){
        return status;
      }
      inline int getId() const{
	      return job_id;
      }
      inline pid_t getPid() const{
        return pid;
      }
      bool stop();
      bool cont();
      void start(Command *cmd);
      void jobWait();
      std::string jobStr(bool verbose) const;
      std::string jobShortStr() const;
      void killJob(bool loud);

      void updateStatus();

      inline void resetStartTime(){
	      start_time = time(NULL);
      }
      inline int getElapsedTime() const{
      	return difftime(time(NULL), start_time);
      }
      bool operator ==(const JobEntry& other){
      		return job_id == other.getId();
      } 
      bool operator !=(const JobEntry& other){
	      return job_id != other.getId();
      }
      friend ostream& operator<<(ostream& os, const JobEntry& job);
      };
 // TODO: Add your data members

  JobsList(){};
  ~JobsList(){};
  void addJob(Command* cmd);
  void printJobsList();
  void printJobsListA(bool verbose);
  void killAllJobs(bool is_loud);
  void removeFinishedJobs();
  void updateAllJobsStatus();
  JobEntry* getJobById(int jobId);
  void stopJobById(int id);
  void removeJobById(int jobId);
  JobEntry* getLastJob();
  JobEntry* getLastStoppedJob();
  // TODO: Add extra methods or modify exisitng ones as needed
  static JobsList& getInstance() // make SmallShell singleton
  {
    static JobsList instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  int getMaxId();
  bool setFg(int job_id){
    JobEntry* j = getJobById(job_id);
    if(j == nullptr){
      return false;
    }
    else{
      fg = j;
      return true;
    }
  }

  bool resetFg(int job_id){
    if (fg->getId() != job_id){
      return false;
    }
    else{
      fg = nullptr;
      return true;
    }
  }
  JobEntry* getFg(){
    return fg;
  }
private:
    JobEntry* fg;
    bool static compareJobs(JobEntry& j1, JobEntry& j2){
        return j1.getId() < j2.getId();
    }
    bool static isFinished(JobEntry& job){
    	return job.getStatus() == Status::finished;
    }

    static const int MAX_JOBS = 100;
    std::vector <JobsList::JobEntry> jobs_list;

};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line);
  virtual ~JobsCommand() {}
  void execute() override;
  //bool validate() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line);
  virtual ~KillCommand() {}
  void execute() override;
  bool validate() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
  bool validate() override {return true;}
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line);
  virtual ~BackgroundCommand() {}
  void execute() override;
  bool validate() override {return true;}
};

class TailCommand : public BuiltInCommand {
private:
    int N;
 public:
  TailCommand(const char* cmd_line);
  virtual ~TailCommand() {}
  void execute() override;
  bool validate() override;
  inline void setNumOfRows(int num) { N = num; };
};

class TouchCommand : public BuiltInCommand {
 public:
  TouchCommand(const char* cmd_line);
  virtual ~TouchCommand() {}
  void execute() override;
  bool validate() override;
};


class SmallShell {
 private:  
  static char* DEFAULT_PROMPT;
  std::string current_prompt;
  ExternalCommand *fg_cmd = nullptr;
  SmallShell();

 public:
  std::vector<std::string> old_path_stack;
  Command *CreateCommand(const char* cmd_line, bool* isExternal);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();  //consider add implementation (for safty)
  void executeCommand(const char* cmd_line);
  /*pid_t inline getPid() const{
	  return getpid();
  }*/
  void inline setPrompt(char* prompt){
    current_prompt = prompt;
  }
  void inline resetPrompt(){
    setPrompt(DEFAULT_PROMPT);
  }


  inline std::string getPrompt() const {
    return current_prompt;
  }

};
//EXTERNAL BRANCH
#endif //SMASH_COMMAND_H_
