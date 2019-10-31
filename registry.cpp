#include "registry.h"

#include <iostream>
#include <sstream>

registry::registry():
registrations{},
mcast_hints{} {
   mcast_hints.ai_family = PF_UNSPEC;
   mcast_hints.ai_socktype = SOCK_DGRAM;
   mcast_hints.ai_flags = AI_NUMERICHOST;
}
bool registry::update(const msg::subscription &msg, const sockaddr_storage& client) {
   auto subscription_src{to_sock_storage(msg.addr, msg.port)};
   if(std::get<0>(subscription_src)) {
      struct sockaddr_storage& addr{std::get<1>(subscription_src)};
      auto itr = registrations.find(addr);
      if(itr != registrations.end()) {
         if(msg.type == msg::message_type::unsubscribe) {
            std::cout << "Client is un-subscribing from " << msg.addr << ":" << msg.port << std::endl;
            itr->second.erase(client);
            if(itr->second.size() == 0) {
               std::cout << "Removing last client for multicast source: " << msg.addr << ":" << msg.port << std::endl;
               registrations.erase(itr);
            }
         }
         else {
            std::set<registration>& regs{itr->second};
            auto client_registration = itr->second.find(client);
            if(client_registration != itr->second.end()) {
               std::cout << "Updating last report time" << std::endl;
               client_registration->renew();
            }
            else {
               std::cout << "Adding new client for existing multicast source " << msg.addr << ":" << msg.port
                         << std::endl;
               itr->second.insert(registration(client));
            }
         }
      }
      else {
         std::cout << "New subscription for " << msg.addr << ":" << msg.port << std::endl;
         registrations.insert(std::make_pair(addr, std::set<registration>{registration(addr)}));
      }
   }

   return false;
}

std::tuple<bool, struct sockaddr_storage> registry::to_sock_storage(const char* mcast_addr, uint16_t port) {
   struct addrinfo* results;
   std::ostringstream ss;
   ss << port;

   int status = getaddrinfo(mcast_addr, ss.str().c_str(), &mcast_hints, &results);
   if(status != 0 || results == nullptr) {
      std::cerr << "Error looking up multicast address: " << mcast_addr << ":" << port << std::endl;
      return std::make_tuple(false, sockaddr_storage{});
   }

   struct sockaddr_storage storage{};
   memset(&storage, 0, sizeof(storage));
   memcpy(&storage, results->ai_addr, results->ai_addrlen);

   return std::make_tuple(true, storage);
}
