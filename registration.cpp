#include "registration.h"

#include <string.h>
#include "constants.h"

namespace chrono = std::chrono;

registration::registration(struct sockaddr_storage client_address, std::chrono::steady_clock::time_point last_reported):
client_addr{client_address},
last_reported{last_reported} {
}

bool registration::expired() const {
   auto now = chrono::steady_clock::now();
   auto duration = chrono::duration_cast<chrono::seconds>(now - last_reported);
   return(duration > constants::HEARTBEAT_INTERVAL * 3);
}

void registration::renew(std::chrono::steady_clock::time_point report_time) const {
   last_reported = report_time;
}

bool registration::operator<(const registration& rhs) const {
   return memcmp(&client_addr, &rhs.client_addr, sizeof(client_addr)) < 0;
}

bool registration::operator<(const struct sockaddr_storage& client) const {
   return memcmp(&client_addr, &client, sizeof(client_addr)) < 0;
}

bool registration::operator==(const struct sockaddr_storage &client) const {
   return memcmp(&client_addr, &client, sizeof(client_addr)) == 0;
}

bool registration::operator==(const struct registration &rhs) const {
   return memcmp(&client_addr, &rhs.client_addr, sizeof(client_addr)) == 0;
}
