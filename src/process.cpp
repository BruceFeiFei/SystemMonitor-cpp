#include "process.h"

#include <unistd.h>

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::cout;
using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) { pid_ = pid; }

int Process::Pid() const { return pid_; }

float Process::CpuUtilization() {
  float total = float(LinuxParser::ActiveJiffies(Pid())) / sysconf(_SC_CLK_TCK);
  float sec = float(LinuxParser::UpTime(Pid()));
  float u =  (total / sec);
  return u;
}

string Process::Command() {
  std::string cmd = LinuxParser::Command(Pid());
  if (cmd.length() >= 50) {
    cmd = cmd.substr(0, 50);
    cmd = cmd + "...";
  }
  return cmd;
}

string Process::Ram() const { return LinuxParser::Ram(Pid()); }

string Process::User() { return LinuxParser::User(Pid()); }

long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

bool Process::operator<(Process const& a) const {
  return stol(Ram()) < stol(a.Ram());
}