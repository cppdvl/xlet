//
// Created by Julian Guarin on 10/01/24.
//
#include "xlet.h"
#include <vector>
#include <cstddef>
#include <iomanip>
#include <iostream>

void printHexy(std::vector<std::byte>&data, int maxrowsize = 32, int groupsize = 4)
{
    int rowsize = 0;
    int address = 0;
    int digitsInAddress = XLET_MAXBLOCKSIZE_SHIFTER % groupsize == 0 ? (XLET_MAXBLOCKSIZE_SHIFTER / groupsize) : (XLET_MAXBLOCKSIZE_SHIFTER / groupsize) + 1;

    auto originalSize = data.size();
    while (data.size() % maxrowsize) data.push_back(std::byte{0});

    std::vector<char> digits{};
    for (auto& byte : data)
    {

        if (!(rowsize % maxrowsize)) std::cout << std::hex << std::setw(digitsInAddress) << std::setfill('0') << (int)address << " ";

        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned char)byte;
        digits.push_back((char)byte);
        if (rowsize % 4 == 0)
        {
            std::cout << " ";
        }
        if ((rowsize % maxrowsize) == (maxrowsize - 1))
        {
            std::cout << " ";
            for (auto& digit : digits)
            {
                if (std::isprint(digit))
                {
                    std::cout << digit;
                }
                else
                {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
            address += maxrowsize;
        }
        else
        {
            std::cout << ".";
        }
        rowsize++;
        if (rowsize == data.size() && rowsize % maxrowsize)
        {

        }
    }
    std::cout << std::endl;
    //go back to original size
    data.resize(originalSize);
}
void handleIncomingData(std::vector<std::byte>& data) {
    // Process incoming data
    std::cout << "Received data of size: " << data.size() << std::endl;
    // Additional processing can be done here
    printHexy(data);
}

void runServer(const std::string& sockpath)
{
        xlet::UDSInOut udsServer(sockpath, true);
    udsServer.setIncomingDataCallBack(handleIncomingData);
    std::thread inboundDataHandlerThread{udsServer.inboundDataHandler};
    if (udsServer.valid()) {
        std::cout << "UDS Server is running using socket: " << sockpath << std::endl;
        // Server's main loop goes here
    } else {
        std::cerr << "Failed to initialize UDS Server" << std::endl;
    }

    inboundDataHandlerThread.join();
}

void runClient(const std::string& sockpath)
{
    xlet::UDSOut udsClient(sockpath);
    if (udsClient.valid()) {
        std::cout << "UDS Client is running using socket: " << sockpath << std::endl;
        std::vector<std::byte> data{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
        udsClient.pushData(data);
    } else {
        std::cerr << "Failed to initialize UDS Client" << std::endl;
    }
}
int main(int argc, char**argv)
{
    std::string sockpath = "/tmp/uds_socket";

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <server|client>" << std::endl;
        return 1;
    }
    std::string mode = argv[1];
    if (mode == "server")
    {
        runServer(sockpath);
    }
    else if (mode == "client")
    {
        runClient(sockpath);
    }
    else
    {
        std::cerr << "Usage: " << argv[0] << " <server|client>" << std::endl;
        return 1;
    }
    return 0;
}