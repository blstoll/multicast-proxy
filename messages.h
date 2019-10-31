#ifndef MULTICAST_PROXY_MESSAGES_H
#define MULTICAST_PROXY_MESSAGES_H

#include <stdint.h>
#include <string.h>

#include <string>
#include <ostream>

namespace msg {
    enum class message_type : uint8_t {subscribe, heartbeat, unsubscribe};

    static std::ostream& operator<<(std::ostream& os, const message_type& type) {
       switch(type) {
          case message_type::subscribe:
             return os << "SUBSCRIBE";
          case message_type::heartbeat:
             return os << "HEARTBEAT";
          case message_type::unsubscribe:
             return os << "UNSUBSCRIBE";
          default:
             return os << "UNKNOWN";
       }
    }

    struct subscription {

        subscription():
        addr{"127.0.0.1"},
        port{36000},
        type{message_type::subscribe}
        {
        }

        subscription(const std::string& ip_addr, uint16_t port, message_type type):
        addr{0},
        port{port},
        type{type}
        {
            strncpy(&addr[0], ip_addr.c_str(), std::min<size_t>(INET6_ADDRSTRLEN -1, ip_addr.size()));
        }

        char addr[INET6_ADDRSTRLEN];
        uint16_t port;
        message_type type;
    };

    static subscription subscribe(const std::string& ip_addr, uint16_t port) {
        return subscription{ip_addr, port, message_type::subscribe};
    }

    static subscription heartbeat(const std::string& ip_addr, uint16_t port) {
        return subscription{ip_addr, port, message_type::heartbeat};
    }

    static subscription unsubscribe(const std::string& ip_addr, uint16_t port) {
        return subscription{ip_addr, port, message_type::unsubscribe};
    }
}

#endif //MULTICAST_PROXY_MESSAGES_H
