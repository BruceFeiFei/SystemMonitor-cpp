#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}


string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return version;
}


vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  float total, free;
  int count = 0;
  string name, line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> name) {
        count++;
        if (name == "MemTotal") {
          linestream >> total;
          break;
        }
        if (name == "MemFree") {
          linestream >> free;
          break;
        }
      }
      if (count == 2) break;
    }
  }
  float res = (total - free) / total;
  long ans = res * 1000;
  res = ans / 1000.0;
  return float(res);
}


long LinuxParser::UpTime() {
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return stol(uptime);
}

long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

long LinuxParser::ActiveJiffies(int pid) {
  string line, value;
  vector<string> values;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.push_back(value);
    }
  }
  return stol(values[13] + values[14]);
}

long LinuxParser::ActiveJiffies() {
  vector<string> cpu_used = LinuxParser::CpuUtilization();
  return stol(cpu_used[CPUStates::kUser_]) + stol(cpu_used[CPUStates::kNice_]) +
         stol(cpu_used[CPUStates::kSystem_]) +
         stol(cpu_used[CPUStates::kIRQ_]) +
         stol(cpu_used[CPUStates::kSoftIRQ_]) +
         stol(cpu_used[CPUStates::kSteal_]);
}

long LinuxParser::IdleJiffies() {
  vector<string> cpu_used = LinuxParser::CpuUtilization();
  return stol(cpu_used[CPUStates::kIdle_]) +
         stol(cpu_used[CPUStates::kIOwait_]);
}

vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpu_used;
  string line, used;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> used;
    while (linestream >> used) {
      cpu_used.push_back(used);
    }
  }
  return cpu_used;
}

int LinuxParser::TotalProcesses() {
  string name, line;
  int totalprocess;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> name) {
        if (name == "processes") {
          linestream >> totalprocess;
          return totalprocess;
        } else {
          break;
        }
      }
    }
  }
  return 0;
}

int LinuxParser::RunningProcesses() {
  string name, line;
  int process_running;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> name) {
        if (name == "procs_running") {
          linestream >> process_running;
          return process_running;
        } else {
          break;
        }
      }
    }
  }
  return 0;
}

string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
}

string LinuxParser::Ram(int pid) {
  string line, key, value;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream stringline(line);
      while (stringline >> key) {
        if (key == "VmSize") {
          stringline >> value;
          return to_string(stol(value) / 1024);
        }
      }
    }
  }
  return "0";
}

string LinuxParser::Uid(int pid) {
  string line, key, value;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream streamline(line);
      while (streamline >> key) {
        if (key == "Uid") {
          streamline >> value;
          return value;
        }
      }
    }
  }
  return string();
}

string LinuxParser::User(int pid) {
  string line, key, value, x;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream streamline(line);
      while (streamline >> key >> x >> value) {
        if (value == LinuxParser::Uid(pid)) {
          return key;
        }
      }
    }
  }
  return string();
}

long LinuxParser::UpTime(int pid) {
  string line, value;
  int count = 0;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      count++;
      if (count == 22) break;
    }
    return LinuxParser::UpTime() - (stol(value) / 100);
  }
  return 0;
}
