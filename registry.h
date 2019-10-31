#ifndef MULTICAST_PROXY_REGISTRY_H
#define MULTICAST_PROXY_REGISTRY_H

#include <map>
#include <set>
#include <tuple>

#include <sys/socket.h>
#include <netdb.h>

#include "registration.h"
#include "messages.h"

namespace {
   struct source_compare_t {
      bool operator()(const struct sockaddr_storage& lhs, const struct sockaddr_storage& rhs) const {
         return memcmp(&lhs, &rhs, sizeof(sockaddr_storage)) < 0;
      }
   };
}

class registry {
public:
   registry();
   bool update(const msg::subscription& msg, const sockaddr_storage& client);

private:
   std::tuple<bool, struct sockaddr_storage> to_sock_storage(const char* mcast_addr, uint16_t port);

   std::map<struct sockaddr_storage, std::set<registration>, source_compare_t> registrations;
   struct addrinfo mcast_hints;
};

#endif //MULTICAST_PROXY_REGISTRY_H
