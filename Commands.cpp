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
#include <wordexp.h>
#include<fstream>


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
 perror("smash error: fork failed");
};

Command::Command(const char* cmd_line) : cmd_line(cmd_line){
	getFgCommandLine(fg_cmd_line);
}
void Command::getFgCommandLine(char* fg_cmd_line){
	strcpy(fg_cmd_line, cmd_line);
	_removeBackgroundSign(fg_cmd_line);
}
	
const int GetCurrDirCommand::MAX_PATH_LENGTH = 1024;
void GetCurrDirCommand::execute(){
  char path_buff[GetCurrDirCommand::MAX_PATH_LENGTH];
  cout << getcwd(path_buff, GetCurrDirCommand::MAX_PATH_LENGTH) << endl;
}

/*void JobsList::JobEntry::kill(){//TODO - REAL IMPLEMENT
  return;
}*/

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

//JobsList::JobEntry(Command* cmd, int job_id){};

JobsList::JobEntry::~JobEntry(){
	//printf("in JE destructor, with pid=%d\n", getpid());
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int jobId){
    int max = -1;
	for (auto itr = jobs_list.begin(); itr != jobs_list.end(); ++itr){
      if (itr->getStatus() == Status::stopped && itr->getId() > max){
        max = itr->getId();
      }
    }
    return getJobById(max);
}

void JobsList::removeJobById(int jobId){
	for(auto itr = jobs_list.begin(); itr != jobs_list.end(); ++itr){
		if(itr->getId() == jobId){
			jobs_list.erase(itr);
		}
	}
  return;
}

void JobsList::killAllJobs(){
  for (auto itr = jobs_list.begin(); itr != jobs_list.end(); ++itr){
    itr->killJob();
}
return;
}



/*void JobsList::stopJobById(int jobId){
=======
    //itr->kill();
  return;
}

void JobsList::printJobsList(){
  for (auto itr = jobs_list.begin(); itr != jobs_list.end(); ++itr){
    std::cout << *itr << std::endl;
}


void JobsList::stopJobById(int jobId){
>>>>>>> 9959c1f (wip built ins)
  getJobById(jobId)->stop();
  return;
}*/

JobsList::JobEntry::JobEntry(const JobEntry &job){
  job_id = job.job_id;
  start_time = job.start_time;
  status = job.status;
  pid = job.pid;
  strcpy(cmd_line, job.cmd_line);
}
JobsList::JobEntry* JobsList::getLastJob(int lastJobId){
  return getJobById(getMaxId());
}

JobsList::JobEntry* JobsList::getJobById(int job_id){
    auto itr = jobs_list.begin();
    while (itr != jobs_list.end()){
      if (itr->getId() == job_id){
        return &(*itr);
      }
     ++itr;
    }
    return nullptr; //TODO - ERROR 
}

/*bool JobsList::JobEntry::start(){
  cmd->prepare()  //consider putting in execute (but remember they've put it under "public")
  cmd->execute()
  fork
}*/



std::string JobsList::JobEntry::jobStr(bool verbose) const {
	return (verbose ? "[" + to_string(job_id) + "] " : "") \
		+ string(cmd_line) + " : " + to_string(pid)\
		+ (verbose ? " " + to_string(getElapsedTime()) +  " secs" : "")\
		+ (verbose && status == Status::stopped ? " (stopped)" : "");
}

ostream& operator<<(ostream& os, const JobsList::JobEntry& job)
{
    os << job.jobStr(true);
    return os;
}

char* SmallShell::DEFAULT_PROMPT = (char*) "smash";
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

  char fg_cmd_line[COMMAND_MAX_LENGTH];
  strcpy(fg_cmd_line, cmd_line);
  _removeBackgroundSign(fg_cmd_line);

  string cmd_s = _trim(string(fg_cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  
  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) {
    return new ChangePromptCommand(cmd_line);
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

void BuiltInCommand::setReqArgsLen(int len){
    req_args_len = len;
}


void ChangePromptCommand::execute(){
  if(args_len == 1){
    SmallShell::getInstance().resetPrompt();
  }
  else{
    SmallShell::getInstance().setPrompt(args[1]);
  }
  
}
//pid_t SmallShell::getPid()
void ShowPidCommand::execute(){
  std::cout << getpid() << endl;
}

bool ChangeDirCommand::validate(){
  if(!validateArgsLen()){
    std::cerr << "smash error: cd: too many arguments \n";
    return false;
  }
  return true;
}
void printPaths(){
	SmallShell &smash = SmallShell::getInstance();
	for (auto itr = smash.old_path_stack.begin(); itr != smash.old_path_stack.end(); ++itr){
		cout << *itr << endl;
	}
	cout << endl;
}
void ChangeDirCommand::execute(){
//    printPaths();
	if (!validate()) {
        return;
    }
    char curr_path[GetCurrDirCommand::MAX_PATH_LENGTH];
    getcwd(curr_path, GetCurrDirCommand::MAX_PATH_LENGTH);

    std::string path = args[1];
    std::string old_path = curr_path;

    char new_path[GetCurrDirCommand::MAX_PATH_LENGTH];
    if (path.compare("-") == 0) {
        if (SmallShell::getInstance().old_path_stack.size() > 0) {
            strcpy(new_path,SmallShell::getInstance().old_path_stack.back().c_str());
            SmallShell::getInstance().old_path_stack.pop_back();
        }
        else {
            return; // TOTO - perror implement
        }
	//printPaths();
    }
    else {
        strcpy(new_path, args[1]);
        SmallShell::getInstance().old_path_stack.push_back(curr_path);
    }
  //prepare();
  if (chdir(new_path) == -1) {
      return; // TOTO - perrror implementation
  };
  //cleanup();
}
bool BuiltInCommand::validateArgsLen(){
    std::cout << args_len;
  return args_len == req_args_len;
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


//void GetCurrDirCommand::execute(){}

void QuitCommand::execute(){}

void JobsCommand::execute(){
	JobsList::getInstance().removeFinishedJobs();
	JobsList::getInstance().printJobsList();
}

void KillCommand::execute(){}

void ForegroundCommand::execute(){}

void BackgroundCommand::execute(){}

static bool isNumber(const std::string& s)
{
    int string_size = s.length();
    for (int i = 0; i < string_size; ++i)
    {
        if (!isdigit(s[i]))
        {
            return false;
        }
    }
    return true;
}

bool TailCommand::validate()
{
    std::string to_validate = args[1];
    std::string validate_if_num = to_validate.substr(1);
    if (!validateArgsLen() && args_len != 3)
    {
        std::cerr << "smash error: tail: invalid arguments \n";
        return false;
    }
    if ((args_len == 3 && to_validate.find_first_of("-") != 0))
    {
        std::cerr << "smash error: tail: invalid arguments \n";
        return false;
    }
    if (isNumber(validate_if_num))
    {
        std::cerr << "smash error: tail: invalid arguments \n";
        return false;
    }
    return true;
}

void TailCommand::execute() {
    if (!validate()) {
        return;
    }
    char* file_name;
    if (args_len == 3) {
        setNumOfRows(std::stoi(std::string(args[1]).substr(1)));
        file_name = args[2];
    }
    else {
        file_name = args[1];
    }
    /*const int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        return; //TOTO - PERROR
    }*/
    std::ifstream file;
    file.open(file_name, std::ifstream::in);
    vector<std::string> rows_q;
    rows_q.resize(N);
    std::string temp_str;
    while (getline(file, temp_str)) {
        if ((signed int)rows_q.size() == N) {
            rows_q.erase(rows_q.begin());
        }
        rows_q.push_back(temp_str.substr(0));
    }
    file.close();
    for (auto itr = rows_q.begin(); itr != rows_q.end(); ++itr) {
        std::cout << *itr << "\n";
    }
}

void TouchCommand::execute(){
    if (!validate()) {
        return;
    }

    char* file_name = args[1];
    std::string raw_format_time = args[2];
    struct tm time = { 0 };
    std::string time_stamp[6];
    int i = 0;

    while (i < 6 && raw_format_time.length()) {
        unsigned int idx = raw_format_time.find_first_of(":");
        time_stamp[i] = raw_format_time.substr(0, idx);
        raw_format_time = raw_format_time.substr(idx + 1);
        ++i;
    }

    time.tm_sec = std::stoi(time_stamp[0]);
    time.tm_min = std::stoi(time_stamp[1]);
    time.tm_hour = std::stoi(time_stamp[2]);
    //reaches here
    time.tm_mday = std::stoi(time_stamp[3]);
    time.tm_mon = (std::stoi(time_stamp[4]) - 1);
    time.tm_year = (std::stoi(time_stamp[5]) - 1900);
    time_t time_stamp_final = mktime(&time);
    struct utimbuf time_buff;
    time_buff.actime = time_stamp_final;
    time_buff.modtime = time_stamp_final;
    if (utime(file_name, &time_buff) == -1) {
        return;
    }
}

bool TouchCommand::validate() {
    if (!validateArgsLen()) {
        std::cerr << "smash error: touch: invalid arguments \n";
        return false;
    }
    return true;
}


void ExternalCommand::execute(){
  //printf("in External::execute\n");
  /*char* execv_arr[] = {BASH, (char*) "-c", fg_cmd_line};
  for(int i=0; i<3;++i){
	  cout << execv_arr[i] << " ";
  }
  cout << endl;*/
  sleep(2);
  execl(BASH,BASH, "-c", fg_cmd_line, (char*) NULL);
  }

void PipeCommand::execute(){};

void RedirectionCommand::execute() {
    int first_redirect_sign = cmd_line.find_first_of(">");
    int last_redirect_sign = cmd_line.find_last_of(">");
    std::string cmd_line = cmd_line;
    std::string command = cmd_line.substr(0, first_redirect_sign);
    std::string output_file = cmd_line.substr(0, last_redirect_sign + 1);
    if (first_redirect_sign != last_redirect_sign)) {
        freopen(output_file, "a", std::stdout);
    }
    else {
        freopen(output_file, "w", std::stdout);
    }
    const char* cmd_format = command.c_str;
    SmallShell::getInstance().executeCommand(cmd_format);
    //freopen(std::cout, "w", output_file);
}

int BuiltInCommand::getLen(char** args){
	int count = 0;
	for(; args[count] != nullptr; ++count);
	return count;
}
static void initializeArr(char** arr, int len){
	for(int i=0; i<len; ++i){
		arr[i] = nullptr;
	}
}

BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line){
  initializeArr(args, COMMAND_MAX_ARGS);
  //char* cmd_line2 = (char*) cmd_line;
//  _removeBackgroundSign(cmd_line2);
  _parseCommandLine(fg_cmd_line, args);
  args_len = getLen(args);
};

ChangePromptCommand::ChangePromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
QuitCommand::QuitCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
JobsCommand::JobsCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
KillCommand::KillCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
ForegroundCommand::ForegroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
BackgroundCommand::BackgroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
TailCommand::TailCommand(const char* cmd_line) : BuiltInCommand(cmd_line), N(10) {
    setReqArgsLen(2);
};
TouchCommand::TouchCommand(const char* cmd_line) : BuiltInCommand(cmd_line){
    setReqArgsLen(3);
};
ShowPidCommand::ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
ChangeDirCommand::ChangeDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
    setReqArgsLen(2);
};
PipeCommand::PipeCommand(const char* cmd_line) : Command(cmd_line){};
RedirectionCommand::RedirectionCommand(const char* cmd_line) : Command(cmd_line) {};

ExternalCommand::ExternalCommand(const char* cmd_line) : Command(cmd_line){
  /*initializeArr(args, COMMAND_MAX_ARGS+1);
  args[0] = BASH;
  char * cmd_line2 = (char*) cmd_line;
  _removeBackgroundSign(cmd_line2);
  _parseCommandLine(cmd_line2, &args[1]);*/
};

//Command::~Command(){};
//JobsList::JobEntry::~JobEntry(){};
/*void QuitCommand::execute(){}

void KillCommand::execute(){}

void ForegroundCommand::execute(){}

void BackgroundCommand::execute(){}

void TailCommand::execute(){}

void TouchCommand::execute(){}
*/


void SmallShell::executeCommand(const char *cmd_line) {
  bool isExternal = false;
  Command* cmd = CreateCommand(cmd_line, &isExternal);
  if (isExternal){
//    int new_id = JobsList::getInstance().getMaxId() + 1;
    JobsList::getInstance().addJob(cmd);
   // printf("finished add job:%s\n",cmd_line);
  }
  else{
    cmd->execute();
    delete cmd;
  }
}
bool JobsList::JobEntry::isFg(){
        return !_isBackgroundComamnd(cmd_line);
      }

int JobsList::getMaxId(){
  if(jobs_list.empty()){
	return 0;  
  }

  int max = 0;
  for (auto itr = jobs_list.begin(); itr != jobs_list.end(); ++itr){
    if (itr->getId() > max){
      max = itr->getId();
    }
  }
  return max;
}

void JobsList::printJobsListA(bool verbose){
  std::sort(jobs_list.begin(),jobs_list.end() , JobsList::compareJobs);
  for(auto itr = jobs_list.begin(); itr != jobs_list.end(); ++itr){
		cout << *itr << (verbose && itr->getStatus() == Status::running ? " (running)" : "") << endl;
	}
}

void JobsList::removeFinishedJobs(){
	updateAllJobsStatus();
	jobs_list.erase(std::remove_if(jobs_list.begin(), jobs_list.end(), isFinished), jobs_list.end());
}
void JobsList::printJobsList(){
	printJobsListA(false);
}
void JobsList::JobEntry::updateStatus(){
	int wstatus;
	if(waitpid(pid, &wstatus, WNOHANG) == 0){
		return;
	}
	if(WIFEXITED(wstatus)){
		status = Status::finished;
	}
	if(WIFSTOPPED(wstatus)){
		status = Status::stopped;
	}
}

void JobsList::updateAllJobsStatus(){
	for (auto it=jobs_list.begin(); it != jobs_list.end(); ++it){
		it->updateStatus();
	}
}
void JobsList::addJob(Command* cmd){
    removeFinishedJobs();
    int new_id = getMaxId() + 1;
    JobEntry job = JobEntry(cmd, new_id);
    delete cmd;
    jobs_list.push_back(job);
    //JobEntry* job2 = getJobById(new_id);
//    cout << "starting job\t:" << *job2 << endl;
//    printf("starting job\tid=%d\t:cmd_line=%s\n",job2->getId(), job2->cmd->getCommandLine());
    getJobById(new_id)->start();
  //  printf("finished starting job\n");
  };

void JobsList::JobEntry::start(){
	pid = fork();
	resetStartTime();
	status = Status::running;
  switch(pid){
    case -1:
      badFork(); //TODO - implement
      break;

    case 0: // in child process
      setpgrp();
      //printf("current process's pid = %d\n",getpid());
      //cout << *this << endl;
      char fg_cmd_line[COMMAND_MAX_LENGTH];
      strcpy(fg_cmd_line, cmd_line);
      _removeBackgroundSign(fg_cmd_line);
      execl(BASH,BASH, "-c", fg_cmd_line, (char*) NULL);
      //cmd->execute();
      //this code is never reached, because cmd->execute calls exec
      status = Status::finished;
      //printf("finished execution in child proccess\n");
      //cout << *this << endl;
    break;

    default: // in main process
    	//sleep(1);
    	//printf("in parent proccess after child started\n");
      bool is_fg = !_isBackgroundComamnd(cmd_line);
      if(is_fg){
	//cout << *this << " is fg" << endl;
        int wstatus;
        waitpid(pid, &wstatus, 0);
	//printf("done waiting, with:status=%d\texited=%d\tstopped=%d\n",WEXITSTATUS(wstatus),WIFSIGNALED(wstatus),WIFSTOPPED(wstatus));
        if(WIFEXITED(wstatus)){
	status = Status::finished;
	}

	else if (WIFSTOPPED(wstatus)){
         stop();
        }
        else{ //assume only get here for terminated or killed child process/command
          /*cout << "deleting cmd" << endl;
		delete cmd;
		*///cout << "assuming job was terminated correctly" << endl;

        }
      }
    break;
    }
};

void JobsList::stopJobById(int id){
  this->getJobById(id)->stop();
};
