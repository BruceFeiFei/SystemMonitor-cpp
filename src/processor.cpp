#include "processor.h"

#include <thread>

#include "linux_parser.h"

float Processor::Utilization() {
  int numCpus = std::thread::hardware_concurrency();
  std::string line, uptime, idletime;
  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
  }
  float usageRatio = (numCpus * std::stol(uptime) - std::stol(idletime));
  return usageRatio / (numCpus * std::stol(uptime));
}