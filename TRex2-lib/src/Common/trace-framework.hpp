//
// Created by espen on 20.11.18.
//

#ifndef TREXSERVER_TRACE_FRAMEWORK_H
#define TREXSERVER_TRACE_FRAMEWORK_H

#include <iostream>
#include "yaml-cpp/yaml.h"

class TraceEvent {
public:
    int locationId;
    std::vector<int> int_arguments;
    std::vector<std::string> string_arguments;
    long long timestamp;
};

void traceEvent(int traceId, const std::vector<int>& int_arguments={}, const std::vector<std::string>& string_arguments={});

void writeBufferToFile();

void setTraceName(std::string tn);

void addTracepointId(int tracepointId);

void setupTraceFramework(const std::vector<int>& tracepointIds, std::string trace_output_folder);

#endif //TREXSERVER_TRACE_FRAMEWORK_H
