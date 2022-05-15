#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	cout << "smash: got ctrl-Z" << endl;
  JobsList::JobEntry* fg_job = JobsList::getInstance().getFg();
  if(fg_job != nullptr){
    fg_job->stop();
  }
  cout << "smash: process " << fg_job->getPid() << "was stopped" << endl;
}

void ctrlCHandler(int sig_num) {
  cout << "smash: got ctrl-C" << endl;
  JobsList::JobEntry* fg_job = JobsList::getInstance().getFg();
  if(fg_job != nullptr){
    fg_job->killJob(true);
  }
  cout << "smash: process " << fg_job->getPid() << "was killed" << endl;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

