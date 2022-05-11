#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <time.h>
#include <utime.h>


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
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif


const std::string WHITESPACE = " \n\r\t\f\v";

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

static void badFork(){
 return;
};

Command::Command(const char* cmd_line) : cmd_line(cmd_line){}
const int GetCurrDirCommand::MAX_PATH_LENGTH = 1024;
void GetCurrDirCommand::execute(){
  char path_buff[GetCurrDirCommand::MAX_PATH_LENGTH];
  printf("%s",getcwd(path_buff, GetCurrDirCommand::MAX_PATH_LENGTH));
}

bool JobsList::JobEntry::stop(){
  if(status != Status::running || kill(pid, SIGTSTP) != 0){
    return false;
  }
  else{
    status = Status::stopped;
    //resetStartTime(); - consider setting here instead of in "cont" (based on instruction in WHW1
    return true;
  }
}

bool JobsList::JobEntry::cont(){
  if(status != Status::stopped || kill(pid, SIGCONT) != 0){
    return false;
  }
  else{
    status = Status::running;
    resetStartTime();  //consider moving to "stop" - based on instructions in WHW1
    return true;
  }
}

/*bool JobsList::JobEntry::start(){
  cmd->prepare()  //consider putting in execute (but remember they've put it under "public")
  cmd->execute()
  fork
}*/

ostream& operator<<(ostream& os, const JobsList::JobEntry& job)
{
    os << '[' << job.job_id << "] " << job.cmd->getCommandLine() <<  " : "  << job.pid\
	    << ' ' << job.getElapsedTime() << " secs";
    if(job.status == Status::stopped){
      os << " (stopped)";
    }
    return os;
}
const std::string SmallShell::DEFAULT_PROMPT = "smash";
SmallShell::SmallShell(){
// TODO: add your implementation
  current_prompt = DEFAULT_PROMPT;
}

SmallShell::~SmallShell() {
// TODO: add your implementation

// ASAF: Nothing to do
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line, bool* isExternal) {
  if (Command::isPipe(cmd_line)){
    return new PipeCommand(cmd_line);
  }
  
  //TADA consider composition of pipe and redirection
  else if (Command::isRedirection(cmd_line)){
    return new RedirectionCommand(cmd_line);
  }


  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  
  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) {
    return new ChangeDirCommand(cmd_line);
  }
  else if (firstWord.compare("quit") == 0) {
    return new QuitCommand(cmd_line);
  }
  else if (firstWord.compare("jobs") == 0) {
    return new JobsCommand(cmd_line);
  }
  else if (firstWord.compare("fg") == 0) {
    return new ForegroundCommand(cmd_line);
  }
  else if (firstWord.compare("bg") == 0) {
    return new BackgroundCommand(cmd_line);
  }
  else if (firstWord.compare("tail") == 0) {
    return new TailCommand(cmd_line);
  }
  else if (firstWord.compare("touch") == 0) {
    return new TouchCommand(cmd_line);
  }

  else {
    *isExternal = true;
    return new ExternalCommand(cmd_line);
  }
  
  return nullptr;
}

void ShowPidCommand::execute(){
  std::cout << SmallShell::getInstance().getPid();
}

bool ChangeDirCommand::validate(){
  if(!validateArgsLen()){
    std::cerr << "smash error: cd: too many arguments";
  }
}
void ChangeDirCommand::execute(){
  if(!validate()){
    return;
  }
  
  prepare();
  
  //consider moving to validate or prepare method
  /*if (sizeof(args)/sizeof(*args) > 1){
    std::cout << "smash error: cd: too many arguments";
  }*/

  /*else if (args[0].compare("-") == 0){
    if (old_dir_path == nullptr){
      	std::cerr << "smash error: cd: OLDPWD not set";
      }
    else{
    	SmallShel::getInstance()->chdir(old_dir_path);
    }
  }*/
      
 /* else if args[0].compare("..") == 0:
    char* curr_path = SmallShel->getInstance()->GetCurrDirCommand::execute();
    std::string str(curr_path);
    old_dir_path = strcpy(str)
    SmallShell::getInstance()->chdir("..");*/
 
    //getcwd(old_dir_path, 0);
  chdir(args[0]);
  cleanup();
}
bool BuiltInCommand::validateArgsLen(){
  return (sizeof(args)/sizeof(*args)) == args_len;
}

bool BuiltInCommand::validate(){
  return true;
}

void BuiltInCommand::prepare(){
  validate();
}
void BuiltInCommand::cleanup(){
  return;
}
void GetCurrDirCommand::execute(){}

void QuitCommand::execute(){}

void JobsCommand::execute(){}

void KillCommand::execute(){}

void ForegroundCommand::execute(){}

void BackgroundCommand::execute(){}

void TailCommand::execute(){}

void TouchCommand::execute(){}


void SmallShell::executeCommand(const char *cmd_line) {
  bool isExternal = false;
  Command* cmd = CreateCommand(cmd_line, &isExternal);
  if (isExternal){
    JobsList::getInstance().addJob(cmd);
    //executeExternal(cmd);
    // deletion of cmd for fg job inside executeExteranl - consider changing because its FUGLY!
  }
  else{
    cmd->execute();
    delete cmd;
  }
}
int JobsList::getMaxId(){
  int max = 0;
  for (auto itr = jobs_list.begin(); itr != jobs_list.end(); ++itr){
    if (itr->getId() > max){
      max = itr->getId();
    }
  }
  return max;
}
void JobsList::addJob(Command* cmd){
    int new_id = getMaxId() + 1;
    JobEntry job = JobEntry(cmd, new_id);
    std::pair<int, JobEntry> p(new_id, job);
    jobs_list.push_back(job);
  };

void JobsList::JobEntry::start(){
	pid_t pid = fork();
	status = Status::running;
  switch(pid){
    case -1:
      badFork(); //TODO - implement
      break;

    case 0: // in child process
      setpgrp();
      cmd->execute();
    break;

    default: // in main process
      resetStartTime();
      bool isFg = !_isBackgroundComamnd(cmd->getCommandLine());
      if(isFg){
        int wstatus;
        waitpid(pid, &wstatus, 0);
        if WIFSTOPPED(wstatus){
         stop();
        }
        else{ //assume only get here for terminated or killed child process/command
          delete cmd;
        }
      }
    break;
    }
};

void JobsList::stopJobById(int id){
  this->getJobById(id)->stop();
};

  
  // Please note that you must fork smash process for some commands (e.g., external commands....)

/*
void SmallShell::executeInternal(Command* cmd) {
  cmd->execute();
}

void SmallShell::executeExternal(Command* ecmd) {
  JobsList::
  ecmd->setPid(fork());  
  switch (ecmd->getPid()){
  case -1:
    bad_fork();  //TADA - add implemenation
    break;
  
  case 0:  // child process
    setpgrp();
    ecmd->execute(); //consider adding prepare & cleanup

  default: //parent process
    int wstatus;
    bool isFg = !_isBackgroundComamnd(ecmd->getCommandLine());
    if(isFg){
      fg_cmd = ecmd;
      waitpid(cmd_pid, *wstatus, 0);
      fg_cmd = nullptr;

      if WIFSTOPPED(wstatus){
        JobsList::getInstance().createStoppedJob(ecmd);
      }

      else{  //assume only get here for terminated or killed child process/command
        delete cmd;
      }
    }

    else{
      JobsList::getInstance().createRunningJob(ecmd);
    }

    break;
  }
  
  }*/
  
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
