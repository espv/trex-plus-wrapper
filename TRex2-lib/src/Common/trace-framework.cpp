//
// Created by espen on 13.12.18.
//


#include <chrono>
#include <sys/types.h>
#include "trace-framework.hpp"

#include <iostream>
#include <fstream>

#include <utility>
// For creating a unique trace file name
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace boost::uuids;

using namespace std;

bool doTrace = false;
std::string trace_name;
pthread_mutex_t* traceMutex = new pthread_mutex_t;

int tracedEvents = 0;
std::string trace_path;

#define MAX_NUMBER_EVENTS 10000000
TraceEvent events[MAX_NUMBER_EVENTS];

std::vector<int> tracepointIDs;

void traceEvent(int traceId, const std::vector<int>& int_arguments, const std::vector<std::string>& string_arguments)
{
  if (!doTrace || std::find(std::begin(tracepointIDs), std::end(tracepointIDs), traceId) == std::end(tracepointIDs))
    return;

  pthread_mutex_lock(traceMutex);
  auto current_time = std::chrono::system_clock::now().time_since_epoch().count();

  if (tracedEvents >= MAX_NUMBER_EVENTS - 1)
    writeBufferToFile();
  events[tracedEvents].locationId = traceId;
  events[tracedEvents].timestamp = current_time;
  events[tracedEvents].int_arguments = int_arguments;
  events[tracedEvents].string_arguments = string_arguments;
  ++tracedEvents;
  pthread_mutex_unlock(traceMutex);
}

void addTracepointId(int tracepointId) {
  tracepointIDs.push_back(tracepointId);
}

int trace_index = 0;
void writeBufferToFile()
{
  std::cout << "Writing trace to file" << std::endl;
  ofstream myfile;
  std::ostringstream oss;
  oss << trace_path;
  std::string file_path = oss.str();
  myfile.open (file_path, fstream::in | fstream::out | fstream::app);
  for (int i = 0; i < tracedEvents; ++i)
  {
    TraceEvent *event = &events[i];

    myfile << event->locationId << "\t" << event->timestamp;
    for (auto arg : event->int_arguments) {
        myfile << "\t" << arg;
    }
    for (const auto& arg : event->string_arguments) {
      myfile << "\t" << arg;
    }
    myfile << "\n";
  }
  std::cout << "Finished writing trace file " << file_path << std::endl;
  myfile.close();

  tracedEvents = 0;
  memset(events, 0, sizeof(TraceEvent)*MAX_NUMBER_EVENTS);
  //exit(0);
}

void setTraceName(std::string tn) {
  trace_name = std::move(tn);
}

void setupTraceFramework(const std::vector<int>& tracepointIds, std::string folder) {
  pthread_mutex_init(traceMutex, nullptr);
  for (int tracepointId : tracepointIds) {
      addTracepointId(tracepointId);
  }
  random_generator gen;
  std::string id = to_string(gen());
  trace_path = folder + "/TRexExperimentFramework-" + std::to_string(std::time(0)) + "-" + id + "-" + trace_name + "-" + std::to_string(trace_index++) + ".trace";
}
