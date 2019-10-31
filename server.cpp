#include <iostream>
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "messages.h"
#include <fcntl.h>
#include <sys/select.h>

#include "constants.h"
#include "registry.h"

int main() {
    int server_fd{-1};
    const uint16_t port{constants::DEFAULT_PORT};

    msg::subscription sub_msg;

    struct sockaddr_in6 server_addr{};
    struct sockaddr_storage client_addr{};

    memset(&server_addr, 0, sizeof(server_addr));

    if((server_fd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(-1);
    }

    if(fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "WARNING: error setting socket to non-blocking..." << std::endl;
    }

    const int ipv6_only_off = 0;
    if(setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only_off, sizeof(ipv6_only_off)) < 0) {
       std::cerr << "WARNING: error disabling IPV6 only on server socket" << std::endl;
    }

    server_addr.sin6_len = sizeof(server_addr);
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);

    if(bind(server_fd, reinterpret_cast<const struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        perror("Error binding to socket");
        shutdown(server_fd, SHUT_RDWR);
        exit(-1);
    }

    fd_set read_fds;
    struct timeval timeout{0, 500000};

    registry reg;

    bool keep_running = true;
    while(keep_running) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;

        auto select_result = select(server_fd + 1, &read_fds, nullptr, nullptr, &timeout);
        if(select_result > 0) {
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_len{sizeof(client_addr)};

            int bytes = recvfrom(server_fd, reinterpret_cast<char*>(&sub_msg), sizeof(sub_msg), 0,
                                 reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);

            std::cout << "Someone sent me a message...." << std::endl;
            if(bytes != sizeof(sub_msg)) {
                std::cerr << "Received a badly formatted packet containing: " << bytes
                          << " bytes, expected: " << sizeof(sub_msg) << std::endl;
            }

            // Ensure address string is null terminated to prevent buffer overflows
            sub_msg.addr[INET6_ADDRSTRLEN - 1] = 0;
            std::cout << "Received " << sub_msg.type << " message for "
                      << sub_msg.addr << ":" << sub_msg.port << std::endl;
            reg.update(sub_msg, client_addr);
        }
        else if(select_result < 0) {
            std::cerr << "Error on subscription interface, port: " << port << " exiting!" << std::endl;
            shutdown(server_fd, SHUT_RDWR);
        }
    }
    return 0;
}