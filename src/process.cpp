#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include<iostream>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;
using std::cout;

Process::Process(int pid) { pid_ = pid; }


int Process::Pid() const { return pid_; }


float Process::CpuUtilization(){
  float total = float(LinuxParser::ActiveJiffies(Pid())) / 100.f;
  float sec = float(LinuxParser::UpTime(Pid()));
  float u = 100*(total / sec / 100.f);
  return u;
}

string Process::Command() { return LinuxParser::Command(Pid()); }

string Process::Ram() const { return LinuxParser::Ram(Pid()); }

string Process::User() { return LinuxParser::User(Pid()); }

long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

bool Process::operator<(Process const& a) const {
  return stol(Ram()) < stol(a.Ram());
}