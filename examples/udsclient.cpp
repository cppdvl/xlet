//
// Created by Julian Guarin on 10/01/24.
//
#include "xlet.h"
#include <iostream>
#include <vector>
#include <cstddef>

int main() {
    std::string sockpath = "/tmp/uds_socket";

    xlet::UDSOut udsClient(sockpath);

    if (udsClient.valid()) {
        std::vector<std::byte> data{/* data bytes here */};
        udsClient.pushData(data);
    } else {
        std::cerr << "Failed to initialize UDS Client" << std::endl;
    }

    return 0;
}
