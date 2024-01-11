//
// Created by Julian Guarin on 10/01/24.
//
#include "xlet.h"
#include <vector>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <functional>

void printHexy(std::vector<std::byte>&data, int maxrow = 16)
{
    int row = 0;
    for (auto& byte : data)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
        if (row++ == maxrow)
        {
            std::cout << std::endl;
            row = 0;
        }
    }
    std::cout << std::endl;
}


void handleIncomingData(std::vector<std::byte>& data) {
    // Process incoming data
    std::cout << "Received data of size: " << data.size() << std::endl;
    // Additional processing can be done here

}

int main() {
    std::string sockpath = "/tmp/uds_socket";

    xlet::UDSInOut udsServer(sockpath, true);
    udsServer.setIncomingDataCallBack(handleIncomingData);

    if (udsServer.valid()) {
        std::cout << "UDS Server is running using socket: " << sockpath << std::endl;
        // Server's main loop goes here
    } else {
        std::cerr << "Failed to initialize UDS Server" << std::endl;
    }

    return 0;
}