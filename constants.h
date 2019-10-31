#ifndef MULTICAST_PROXY_CONSTANTS_H
#define MULTICAST_PROXY_CONSTANTS_H

#include <chrono>
#include <stdint.h>

namespace constants {
   static constexpr std::chrono::seconds HEARTBEAT_INTERVAL = std::chrono::seconds{5};
   static constexpr uint16_t DEFAULT_PORT = 32577;
}

#endif //MULTICAST_PROXY_CONSTANTS_H
