//
// async_tcp_client.cpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "tcpclient.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include "../../../TRex-Server/src/external.hpp"
#include "../Packets/PubPkt.h"

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

//
// This class manages socket timeouts by applying the concept of a deadline.
// Some asynchronous operations are given deadlines by which they must complete.
// Deadlines are enforced by an "actor" that persists for the lifetime of the
// TcpClient object:
//
//  +----------------+
//  |                |
//  | check_deadline |<---+
//  |                |    |
//  +----------------+    | async_wait()
//              |         |
//              +---------+
//
// If the deadline actor determines that the deadline has expired, the socket
// is closed and any outstanding operations are consequently cancelled.
//
// Connection establishment involves trying each endpoint in turn until a
// connection is successful, or the available endpoints are exhausted. If the
// deadline actor closes the socket, the connect actor is woken up and moves to
// the next endpoint.
//
//  +---------------+
//  |               |
//  | start_connect |<---+
//  |               |    |
//  +---------------+    |
//           |           |
//  async_-  |    +----------------+
// connect() |    |                |
//           +--->| handle_connect |
//                |                |
//                +----------------+
//                          :
// Once a connection is     :
// made, the connect        :
// actor forks in two -     :
//                          :
// an actor for reading     :       and an actor for
// inbound messages:        :       sending heartbeats:
//                          :
//  +------------+          :          +-------------+
//  |            |<- - - - -+- - - - ->|             |
//  | start_read |                     | start_write |<---+
//  |            |<---+                |             |    |
//  +------------+    |                +-------------+    | async_wait()
//          |         |                        |          |
//  async_- |    +-------------+       async_- |    +--------------+
//   read_- |    |             |       write() |    |              |
//  until() +--->| handle_read |               +--->| handle_write |
//               |             |                    |              |
//               +-------------+                    +--------------+
//
// The input actor reads messages from the socket, where messages are delimited
// by the newline character. The deadline for a complete message is 30 seconds.
//
// The heartbeat actor sends a heartbeat (a message that consists of a single
// newline character) every 10 seconds. In this example, no deadline is applied
// message sending.
//

TcpClient *TcpClient::start_connection(const std::map<std::string, std::string> address)
{
  try
  {
    auto *io_service = new boost::asio::io_service();
    auto *work = new boost::asio::io_service::work(*io_service);
    auto *c = new TcpClient(*io_service);
    c->r = new tcp::resolver(*io_service);
    c->io_service = io_service;

    c->address = address;
    c->start(c->r->resolve(tcp::resolver::query(address.at("ip"), address.at("port"))));

    return c;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return nullptr;
}

void TcpClient::doSend() {
  boost::system::error_code ec;
  boost::asio::async_write(socket_, boost::asio::buffer(marshalled_pkts->front()->second, marshalled_pkts->front()->first),
                           boost::bind(&TcpClient::handle_write, this, ec));
}

void TcpClient::sendPubPkt(std::pair<std::size_t, std::vector<char>> *marshalled_pkt) {
  pthread_mutex_lock(busy_mutex);
  marshalled_pkts->push_back(marshalled_pkt);
  if (busy) {
    pthread_mutex_unlock(busy_mutex);
    return;
  }
  busy = true;
  pthread_mutex_unlock(busy_mutex);
  doSend();
}

void TcpClient::restart_connection() {
  try
  {
    this->start(this->r->resolve(tcp::resolver::query(address["ip"], address["port"])));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}
