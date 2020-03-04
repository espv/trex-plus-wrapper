//
// Created by espen on 14.12.2019.
//

#ifndef T_REX_CLIENT_H
#define T_REX_CLIENT_H

#include <utility>
#include <vector>

class Client {
public:
    pthread_mutex_t* busy_mutex = new pthread_mutex_t;
    Client() {
      pthread_mutex_init(busy_mutex, NULL);
    }

    virtual void sendPubPkt(std::pair<std::size_t, std::vector<char>> *marshalled_pkt) = 0;
    static Client *start_connection(std::map<std::string, std::string> address);
    virtual void restart_connection() = 0;
    virtual bool has_stopped() = 0;
};

#endif //T_REX_CLIENT_H
