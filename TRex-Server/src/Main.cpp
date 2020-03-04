/*
 * Copyright (C) 2011 Francesco Feltrinelli <first_name DOT last_name AT gmail DOT com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "external.hpp"
#include "../../TRex2-lib/src/Common/trace-framework.hpp"
#include "../../TRex2-lib/src/Engine/TRexEngine.h"
#include "Packet/BufferedPacketUnmarshaller.hpp"
#include <memory>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <cstring>
#include <queue>
#include "experiment-framework/experimentapi.h"

using concept::server::SOEPServer;
using namespace concept::test;
using concept::util::Logging;
using namespace std;

#define SEND_PACKETS_FOREVER

bool *cont = new bool();
SOEPServer *server;
void signal_handler(int s){
  *cont = false;
  server->stop();
	//writeBufferToFile();
}

extern bool doTrace;
int main(int argc, char* argv[]){
  auto * experimentAPI = new ExperimentAPI();
  bool client = false;
  std::string master_port;
	for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--trace"))
      doTrace = true;
    else if (!strcmp(argv[i], "--number-threads")) {
      ++i;
      experimentAPI->number_threads = std::stoi(argv[i]);
    } else if (!strcmp(argv[i], "--client-ip")) {
      ++i;
      experimentAPI->client_ip = argv[i];
    } else if (!strcmp(argv[i], "--client-port")) {
      ++i;
      client = true;
      experimentAPI->client_port = argv[i];
    } else if (!strcmp(argv[i], "--trace-output-folder")) {
      ++i;
      experimentAPI->trace_output_folder = argv[i];
      doTrace = true;
    } else if (!strcmp(argv[i], "--vldb-tweak-for-experiment4")) {
      experimentAPI->vldb_tweak_for_experiment4 = true;
    } else if (!strcmp(argv[i], "--coordinator-ip")) {
      ++i;
      experimentAPI->coordinator_ip = argv[i];
    } else if (!strcmp(argv[i], "--coordinator-port")) {
      ++i;
      experimentAPI->coordinator_port = argv[i];
    } else if (!strcmp(argv[i], "--node-id")) {
      ++i;
      experimentAPI->node_id = std::stoi(argv[i]);
    }
  }

	struct sigaction sigIntHandler{};

	sigIntHandler.sa_handler = signal_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, nullptr);

	Logging::init();

  server = new SOEPServer(std::stoi(experimentAPI->client_port), experimentAPI->number_threads, false, false);
  std::cout << "Using " << experimentAPI->number_threads << " threads" << std::endl;
	experimentAPI->this_engine = &server->getEngine();
  new boost::thread(&SOEPServer::run, server);
  //while (true) {
  *cont = true;
  experimentAPI->cont = cont;
  experimentAPI->RunExperiment();
  //}
}
