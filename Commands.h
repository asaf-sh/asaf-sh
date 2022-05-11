#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <time.h>
#include <signal.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using namespace std;
class Command {
// TODO: Add your data members
private:
  const char* cmd_line;
  const char* cmd_name;
 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed

  inline static bool isExternal();

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
  private:
  const char** args;
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
  inline static bool isExternal(){return false;}
};

class ExternalCommand : public Command {
private:
  const char** args;

  
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
  //void inline setPid(pid_t pid) {pid = pid;}
  //pid_t inline getPid() const {return pid;}
};

class PipeCommand : public Command {
 private:
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
  inline static bool isExternal(){return false;}
};

class RedirectionCommand : public Command {
 private:
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  inline static bool isExternal() {return false;}
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  ChangeDirCommand(const char* cmd_line);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
  static const int MAX_PATH_LENGTH;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
  QuitCommand(const char* cmd_line);
  virtual ~QuitCommand() {}
  void execute() override;
};

enum class Status {running, stopped, killed, finished};
class JobsList{
public:
  class JobEntry {
    private:
      const Command* cmd;
      const int job_id;
      time_t start_time;
      pid_t pid;
      Status status;
    public:
      JobEntry(Command* cmd, int job_id, Status status): cmd(cmd), job_id(job_id), status(Status::running){};
      JobEntry(const JobEntry& job) = default; 
      ~JobEntry();
      
      Status getStatus() const;
      bool stop();
      bool cont();
      bool start();
      inline void resetStartTime(){
	      start_time = time(NULL);
      }
      inline int getElapsedTime() const{
      	return difftime(time(NULL), start_time);
      }

      friend ostream& operator<<(ostream& os, const JobEntry& job);
      };
 // TODO: Add your data members

  JobsList();
  ~JobsList();
  void addJob(Command* cmd);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
  bool createJob();
  static JobsList& getInstance() // make SmallShell singleton
  {
    static JobsList instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
private:
    static const int MAX_JOBS = 100;
    std::vector <JobsList::JobEntry> jobs_list;

};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TailCommand : public BuiltInCommand {
 public:
  TailCommand(const char* cmd_line);
  virtual ~TailCommand() {}
  void execute() override;
};

class TouchCommand : public BuiltInCommand {
 public:
  TouchCommand(const char* cmd_line);
  virtual ~TouchCommand() {}
  void execute() override;
};


class SmallShell {
 private:  
  static const std::string DEFAULT_PROMPT;
  std::string current_prompt;
  ExternalCommand *fg_cmd = nullptr;
  SmallShell();

 public:
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
  void executeInternal(Command* cmd);
  void executeExternal(Command* cmd);

  inline std::string getPrompt() const {
    return current_prompt;
  }

};

#endif //SMASH_COMMAND_H_
