//
// Created by Julian Guarin on 10/01/24.
//
#include "xlet.h"
#include <set>
#include <queue>
#include <iomanip>
#include <sstream>


std::set<uint64_t> connectedClients{};

//Top of the queue is the host.
std::queue<uint64_t> orderToHost{};




void log(std::string letId, const std::string& msg)
{
    std::cout << "[ " << letId << " ] : " << msg << std::endl;
}

void printHexy(std::vector<std::byte>&data, int maxrowsize = 16, int groupsize = 4)
{
    int rowsize = 0;
    int address = 0;
    int digitsInAddress = XLET_MAXBLOCKSIZE_SHIFTER % groupsize == 0 ? (XLET_MAXBLOCKSIZE_SHIFTER / groupsize) : (XLET_MAXBLOCKSIZE_SHIFTER / groupsize) + 1;

    auto originalSize = data.size();
    while (data.size() % maxrowsize) data.push_back(std::byte{0});

    std::vector<char> digits{};
    for (auto& byte : data)
    {

        if (!(rowsize % maxrowsize)) {
            digits.clear();
            std::cout << std::hex << std::setw(digitsInAddress) << std::setfill('0') << (int)address << " ";
        }

        if (rowsize % 4 == 0)
        {
            std::cout << " ";
        }
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned long long)byte;
        digits.push_back((char)byte);

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
        else if ((rowsize % groupsize) == (groupsize - 1))
        {
            std::cout << " ";
        }
        else
        {
            std::cout << ".";
        }
        rowsize++;

    }
    std::cout << std::endl;
    //go back to original size
    data.resize(originalSize);
}
void handleServerIncomingData(uint64_t peerId, std::vector<std::byte>& data) {
    // Process incoming data
    log(xlet::UDPlet::letIdToIpString(peerId), "Received data of size: " + std::to_string(data.size()));
    printHexy(data);
}

void newConnection(uint64_t peerId)
{
    log(xlet::UDPlet::letIdToIpString(peerId), "New connection");

    if (!connectedClients.empty() || !orderToHost.empty()) connectedClients.insert(peerId);
    orderToHost.push(peerId);
}


void runServerRouting(const std::string& ip, int port)
{
    xlet::UDPInOut udpServer(ip, 8899, true);

    udpServer.letBindedOn.Connect(+[](uint64_t letId, std::__thread_id id){
        std::stringstream ss; ss << "UDP Server binded and waiting for data on thread: " << id;
        log(xlet::UDPlet::letIdToString(letId) , ss.str());
    });
    udpServer.enableQueueManagement();
    auto qThread = std::function<void()>([&udpServer](){
        while(true) {

            if (udpServer.qin_.size() > 0) {

                //If Check peer is in connectedClients.
                auto peerIdData = udpServer.qin_.front();
                udpServer.qin_.pop();

                //Get peerId
                auto peerId = peerIdData.first;

                //Check if it's a new connection. If so, add it to the connectedClients set.
                if (connectedClients.find(peerId) == connectedClients.end()) newConnection(peerId);
                auto hostId = orderToHost.front();

                //Broadcast to all connected clients
                if (hostId == peerId) for (auto& client : connectedClients)  udpServer.pushData(client, peerIdData.second);
                //Route data to host.
                else udpServer.pushData(hostId, peerIdData.second);
            }
        }
    });

    std::thread inboundDataHandlerThread(udpServer.inboundDataHandlerThread);
    std::thread routerThread(qThread);

    if (udpServer.valid()) {
        std::cout << "UDP Server is running using socket: " << ip << std::endl;
    } else {
        std::cerr << "Failed to initialize UDP Server" << std::endl;
    }

    inboundDataHandlerThread.join();

}

int main(int argc, char**argv)
{

    runServerRouting("127.0.0.1", 8899);
    return 0;

}
