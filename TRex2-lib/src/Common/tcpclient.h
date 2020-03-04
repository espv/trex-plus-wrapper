//
// Created by espen on 13.12.2019.
//

#ifndef T_REX_TCPCLIENT_H
#define T_REX_TCPCLIENT_H

#include "tcpclient.h"
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <map>
#include "../../../TRex-Server/src/external.hpp"

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

class TcpClient : public Client
{
private:
    bool stopped_;
    boost::asio::streambuf input_buffer_;
    //deadline_timer deadline_;
    //deadline_timer heartbeat_timer_;
    int node_id;
    std::map<std::string, std::string> address;
    std::deque<std::pair<std::size_t, std::vector<char>>*> *marshalled_pkts = new std::deque<std::pair<std::size_t, std::vector<char>>*>();
    bool busy = false;

    void doSend();

    void start_connect(tcp::resolver::iterator endpoint_iter)
    {
      if (endpoint_iter != tcp::resolver::iterator())
      {
        std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

        // Set a deadline for the connect operation.
        //deadline_.expires_from_now(boost::posix_time::seconds(60));

        // Start the asynchronous connect operation.
        socket_.async_connect(endpoint_iter->endpoint(),
                              boost::bind(&TcpClient::handle_connect,
                                          this, _1, endpoint_iter));
      }
      else
      {
        // There are no more endpoints to try. Shut down the TcpClient.
        stop();
      }
    }

    void handle_connect(const boost::system::error_code& ec,
                        tcp::resolver::iterator endpoint_iter)
    {
      if (stopped_)
        return;

      // The async_connect() function automatically opens the socket at the start
      // of the asynchronous operation. If the socket is closed at this time then
      // the timeout handler must have run first.
      if (!socket_.is_open())
      {
        std::cout << "Connect timed out\n";

        // Try the next available endpoint.
        start_connect(++endpoint_iter);
      }

        // Check if the connect operation failed before the deadline expired.
      else if (ec)
      {
        std::cout << "Connect error: " << ec.message() << "\n";

        // We need to close the socket used in the previous connection attempt
        // before starting a new one.
        socket_.close();

        // Try the next available endpoint.
        start_connect(++endpoint_iter);
      }

        // Otherwise we have successfully established a connection.
      else
      {
        std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";

        // Start the input actor.
        //start_read();

        // Start the heartbeat actor.
        //start_write();
      }
    }

    void start_read()
    {
      // Set a deadline for the read operation.
      //deadline_.expires_from_now(boost::posix_time::seconds(30));

      // Start an asynchronous operation to read a newline-delimited message.
      boost::asio::async_read_until(socket_, input_buffer_, '\n',
                                    boost::bind(&TcpClient::handle_read, this, _1));
    }

    void handle_read(const boost::system::error_code& ec)
    {
      if (stopped_)
        return;

      if (!ec)
      {
        // Extract the newline-delimited message from the buffer.
        std::string line;
        std::istream is(&input_buffer_);
        std::getline(is, line);

        // Empty messages are heartbeats and so ignored.
        if (!line.empty())
        {
          std::cout << "Received: " << line << "\n";
        }

        start_read();
      }
      else
      {
        std::cout << "Error on receive: " << ec.message() << "\n";

        stop();
      }
    }

    void start_write()
    {
      if (stopped_)
        return;

      // Start an asynchronous operation to send a heartbeat message.
      boost::asio::async_write(socket_, boost::asio::buffer("\n", 1),
                               boost::bind(&TcpClient::handle_write, this, _1));
    }

    void handle_write(const boost::system::error_code& ec)
    {
      if (stopped_) {
        std::cout << "Stopped" << std::endl;
        return;
      }
      if (ec) {
        std::cout << "Error on sending packet: " << ec.message();
        stop();
      } else {
        //std::cout << "Successfully sent PubPkt to destination" << std::endl;
      }

      pthread_mutex_lock(busy_mutex);
      delete marshalled_pkts->front();
      marshalled_pkts->pop_front();
      pthread_mutex_unlock(busy_mutex);
      if (marshalled_pkts->empty()) {
        //std::cout << "Not busy anymore" << std::endl;
        pthread_mutex_lock(busy_mutex);
        busy = false;
        pthread_mutex_unlock(busy_mutex);
      } else {
        //std::cout << "Sending pkt from handle_write" << std::endl;
        doSend();
      }

      /*if (stopped_)
        return;

      if (!ec)
      {
        // Wait 10 seconds before sending the next heartbeat.
        heartbeat_timer_.expires_from_now(boost::posix_time::seconds(10));
        heartbeat_timer_.async_wait(boost::bind(&TcpClient::start_write, this));
      }
      else
      {
        std::cout << "Error on heartbeat: " << ec.message() << "\n";

        stop();
      }*/
    }

    void check_deadline()
    {
      if (stopped_)
        return;

      // Check whether the deadline has passed. We compare the deadline against
      // the current time since a new asynchronous operation may have moved the
      // deadline before this actor had a chance to run.
      /*if (deadline_.expires_at() <= deadline_timer::traits_type::now())
      {
        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled.
        socket_.close();

        // There is no longer an active deadline. The expiry is set to positive
        // infinity so that the actor takes no action until a new deadline is set.
        deadline_.expires_at(boost::posix_time::pos_infin);
      }*/

      // Put the actor back to sleep.
      //this->deadline_.async_wait(boost::bind(&TcpClient::check_deadline, this));
    }

public:
    tcp::socket socket_;
    boost::asio::io_service *io_service;
    tcp::resolver *r;

    TcpClient(boost::asio::io_service& io_service)
            : stopped_(false),
              socket_(io_service)//,
              //deadline_(io_service),
              //heartbeat_timer_(io_service)
    {
    }

    void sendPubPkt(std::pair<std::size_t, std::vector<char>> *marshalled_pkt) override;

    static TcpClient *start_connection(std::map<std::string, std::string> address);

    void restart_connection() override;

    bool has_stopped() override {return stopped_;}

    // Called by the user of the TcpClient class to initiate the connection process.
    // The endpoint iterator will have been obtained using a tcp::resolver.
    void start(tcp::resolver::iterator endpoint_iter)
    {
      // Start the connect actor.
      start_connect(endpoint_iter);

      // Start the deadline actor. You will note that we're not setting any
      // particular deadline here. Instead, the connect and input actors will
      // update the deadline prior to each asynchronous operation.
      //deadline_.async_wait(boost::bind(&TcpClient::check_deadline, this));
    }

    // This function terminates all the actors to shut down the connection. It
    // may be called by the user of the TcpClient class, or by the class itself in
    // response to graceful termination or an unrecoverable error.
    void stop()
    {
      stopped_ = true;
      boost::system::error_code ignored_ec;
      socket_.close(ignored_ec);
      //deadline_.cancel();
      //heartbeat_timer_.cancel();
    }

    int get_node_id() { return node_id; }
    void set_node_id(int node_id) { this->node_id = node_id; }
};


#endif //T_REX_TCPCLIENT_H
