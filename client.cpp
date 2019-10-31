#include <iostream>
#include <vector>
#include <chrono>

#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "messages.h"
#include "constants.h"

static constexpr size_t MAX_PACKET_SIZE = 9000;

namespace chrono = std::chrono;

std::vector<struct addrinfo> lookup_server(const char* server, uint16_t port) {
   struct addrinfo hints{};
   struct addrinfo* matches;
   std::ostringstream ss;
   ss << port;

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_protocol = IPPROTO_UDP;

   std::vector<struct addrinfo> results;

   auto error = getaddrinfo(server, ss.str().c_str(), &hints, &matches);
   if(error || matches == nullptr) {
      std::cerr << "No result found for server: " << server << ", error: " << gai_strerror(error) << std::endl;
      return results;
   }

   for(auto match = matches; match != nullptr; match = match->ai_next) {
      // Return the first match
      char ip_str[INET6_ADDRSTRLEN];
      memset(ip_str, 0, INET6_ADDRSTRLEN);

      inet_ntop(match->ai_family, match->ai_addr->sa_data, ip_str, INET6_ADDRSTRLEN);
      std::cout << "Resolved: " << ip_str << " for " << server << std::endl;
      results.push_back(*match);
   }
   return results;
}

int main() {
   int fd{-1};
   uint16_t port{constants::DEFAULT_PORT};
   std::vector<struct addrinfo> results{lookup_server("localhost", port)};

   if(results.empty()) {
      exit(-1);
   }

   for(auto result : results) {
      if((fd = socket(result.ai_family, result.ai_socktype, result.ai_protocol)) < 0) {
         perror("Socket creation failed");
         exit(-1);
      }

      if(connect(fd, result.ai_addr, result.ai_addrlen) < 0) {
         std::cerr << "Connection error " << std::endl;
         shutdown(fd, SHUT_RDWR);
         fd = -1;
      }
      else {
         break;
      }
   }

   if(fd < 0) {
      std::cerr << "Unable to resolve a destination to the server. Exiting" << std::endl;
      exit(-1);
   }

   msg::subscription sub_msg = msg::subscribe("224.0.0.0", 3500);
   msg::subscription heartbeat = msg::heartbeat("224.0.0.0", 3500);
   struct sockaddr_storage producer_addr{};
   memset(&producer_addr, 0, sizeof(producer_addr));


#if 0
   if((fd = socket(dest.ai_family, dest.ai_socktype, dest.ai_protocol)) < 0) {
      perror("Socket creation failed");
      exit(-1);
   }

   if(connect(fd, dest.ai_addr, dest.ai_addrlen) < 0) {
      std::cerr << "Connection error " << std::endl;
   }
#endif

   std::cout << "My file descriptor is: " << fd << std::endl;

   if(fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
      std::cerr << "WARNING: error setting socket to non-blocking..." << std::endl;
   }

   fd_set read_fds;
   fd_set write_fds;
   struct timeval timeout{0, 500000};

   std::vector<uint8_t> buffer(MAX_PACKET_SIZE);


   auto last_heartbeat = chrono::steady_clock::now();

   int bytes_sent = send(fd, &sub_msg, sizeof(sub_msg), 0);
   if(bytes_sent != sizeof(sub_msg)) {
      std::cout << "That's weird... I sent " << bytes_sent << " bytes, expected " << sizeof(sub_msg) << std::endl;
   }

   bool keep_running = true;
   while(keep_running) {
      FD_ZERO(&write_fds);
      FD_SET(fd, &write_fds);

      timeout.tv_sec = 0;
      timeout.tv_usec = 500000;

      auto select_result = select(fd + 1, &read_fds, &write_fds, nullptr, &timeout);
      if(select_result > 0) {
         memset(&producer_addr, 0, sizeof(producer_addr));
         socklen_t producer_len{sizeof(producer_addr)};

         if(FD_ISSET(fd, &read_fds)) {
            std::cout << "I have proxied the data...." << std::endl;
            int bytes = recvfrom(fd, reinterpret_cast<char *>(&sub_msg), sizeof(sub_msg), 0,
                                 reinterpret_cast<struct sockaddr *>(&producer_addr), &producer_len);

            if(bytes < 0) {
               std::cerr << "Socket read error  " << bytes << std::endl;
            }
         }
      } else if(select_result == 0) {
         std::cout << "And we wait..." << std::endl;
      } else {
         std::cerr << "Error on subscription interface, port: " << port << " exiting!" << std::endl;
         shutdown(fd, SHUT_RDWR);
      }

      auto now = chrono::steady_clock::now();
      if(chrono::duration_cast<chrono::seconds>(now - last_heartbeat) > constants::HEARTBEAT_INTERVAL) {
         bytes_sent = send(fd, &heartbeat, sizeof(heartbeat), 0);
         if(bytes_sent != sizeof(heartbeat)) {
            std::cout << "That's weird... I sent " << bytes_sent << " bytes, expected "
                      << sizeof(heartbeat) << std::endl;
         }
         std::cout << "Next heartbeat: " << last_heartbeat.time_since_epoch().count() << std::endl;
         last_heartbeat += constants::HEARTBEAT_INTERVAL;
      }
   }

   if(fd >= 0) {
      shutdown(fd, SHUT_RDWR);
   }
   std::cout << "Shutting down subscription listener on port: " << port << std::endl;

   return 0;
}
