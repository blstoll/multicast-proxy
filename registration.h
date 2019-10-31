#ifndef MULTICAST_PROXY_REGISTRATION_H
#define MULTICAST_PROXY_REGISTRATION_H

#include <sys/socket.h>
#include <chrono>

class registration {
public:
   registration(struct sockaddr_storage client_address,
         std::chrono::steady_clock::time_point last_reported = std::chrono::steady_clock::now());
   registration(const registration& rhs) = default;
   registration(registration&& rhs) = default;

   ~registration() = default;

   registration& operator=(const registration& rhs) = default;
   registration& operator=(registration&& rhs) = default;

   bool expired() const;
   void renew(std::chrono::steady_clock::time_point = std::chrono::steady_clock::now()) const;
   bool operator<(const registration& rhs) const;
   bool operator<(const struct sockaddr_storage& client) const;

   bool operator==(const struct registration& rhs) const;
   bool operator==(const struct sockaddr_storage& client) const;
private:
   struct sockaddr_storage client_addr;
   mutable std::chrono::steady_clock::time_point last_reported; // Mutable to allow inclusion in std::set
};

#endif //MULTICAST_PROXY_REGISTRATION_H
