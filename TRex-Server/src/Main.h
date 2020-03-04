//
// Created by espen on 13.12.2019.
//

#ifndef T_REX_MAIN_H
#define T_REX_MAIN_H

#include "yaml-cpp/yaml.h"

void handleEvent(YAML::Node event);

void Configure();

void SetTupleBatchSize(int bs);

void SetIntervalBetweenTuples(int iw);

void AddTuples(YAML::Node tuple, int quantity);

void AddDataset(YAML::Node dataset_to_use);

void ProcessDataset(YAML::Node dataset_to_use);

void AddQueries(YAML::Node query);

void ProcessTuples(int number_tuples);

void ClearQueries();

void ClearTuples();

void AddStreamSchema(YAML::Node stream_definition);

void RunEnvironment();

void StopEnvironment();

void AddSubscriberOfStream(int schema_id, int node_id);

void SetNodeIdToAddress(YAML::Node nodeIdToAddress);

#endif //T_REX_MAIN_H
