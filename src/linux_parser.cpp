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

template <typename T>
T getValueOfFile(std::string const &filename) {
  std::string line;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
  }
  return value;
};
template <typename T>
T findValueByKey(std::string const &keyFilter, std::string const &filename) {
  std::string line, key;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyFilter) {
          return value;
        }
      }
    }
  }
  return value;
};

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
  stream.close();
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR *directory = opendir(kProcDirectory.c_str());
  struct dirent *file;
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
  std::string memTotal = "MemTotal:";
  std::string memFree = "MemFree:";
  float Total = findValueByKey<float>(memTotal, kMeminfoFilename);
  float Free = findValueByKey<float>(memFree, kMeminfoFilename);
  return (Total - Free) / Total;
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
  stream.close();
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
      values.emplace_back(value);
    }
  }
  stream.close();
  return stol(values[13]) + stol(values[14]);
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
  stream.close();
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
  stream.close();
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
          stream.close();
          return process_running;
        } else {
          break;
        }
      }
    }
  }
  stream.close();
  return 0;
}

string LinuxParser::Command(int pid) {
  return std::string(
      getValueOfFile<std::string>(std::to_string(pid) + kCmdlineFilename));
}

string LinuxParser::Ram(int pid) {
  string line, key, value;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream stringline(line);
      while (stringline >> key) {
        // Using VmRSS instead of VmSize. VmSize is the sum of all the virtual
        // memory, it will give us more than the real Physical RAM size, the
        // VmRSS will give us exact physical memory being used as a part of
        // Physical RAM.
        if (key == "VmRSS") {
          stringline >> value;
          stream.close();
          return to_string(stol(value) / 1024);
        }
      }
    }
  }
  stream.close();
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
          stream.close();
          return value;
        }
      }
    }
  }
  stream.close();
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
          stream.close();
          return key;
        }
      }
    }
  }
  stream.close();
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
    stream.close();
    return LinuxParser::UpTime() - sysconf(stol(value));
  }
  stream.close();
  return 0;
}
