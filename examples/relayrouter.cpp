//
// Created by Julian Guarin on 10/01/24.
//
#include "xlet.h"
#include <set>
#include <iomanip>
#include <sstream>


std::set<uint64_t> authorizedPeers{};
std::set<uint64_t> nonHostingPeers{};
static uint64_t sHostId = 0;

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
    //Rule 1. if peerId is not in the group of authorized peers, then reject the connection.
    auto ipString = xlet::UDPlet::letIdToIpString(peerId);
    if (!authorizedPeers.contains(peerId))
    {
        log(ipString, "newConnection: Unauthorized connection");
        return;
    }

    //Rule 2. if peerId is already connected, then ignore the connection.
    if (nonHostingPeers.contains(peerId) || sHostId == peerId)
    {
        log(ipString, "newConnection: Already connected");
        if (sHostId == peerId) log(ipString, "newConnection: attempted Peer is the already host");
        return;
    }

    //Rule 3. If hostId is not connected
    log(ipString, "newConnection: New connection");
    if (sHostId == 0)
    {
        sHostId = peerId;
        log(ipString, "newConnection: New host");
    }
    else
    {
        log(ipString, "newConnection: New client");
        nonHostingPeers.insert(peerId);
    }
}

void purgeConnections()
{
    log("-- x --", "purgeConnections: Purging connections");
    sHostId = 0;
    nonHostingPeers.clear();
    authorizedPeers.clear();
}

void removeConnection(uint64_t peerId)
{
    if (sHostId == peerId) sHostId = 0;
    else if (nonHostingPeers.contains(peerId)) nonHostingPeers.erase(peerId);
    else {
        log(xlet::UDPlet::letIdToIpString(peerId), "removeConnection: Peer not found");
        return;
    }

    log(xlet::UDPlet::letIdToIpString(peerId), "removeConnection: Peer removed");
    log(xlet::UDPlet::letIdToIpString(peerId), "removeConnection: Host removed");
}


void runServerRouting(const std::string& ip, int port)
{
    xlet::UDPInOut udpServer(ip, port, true);

    udpServer.letBindedOn.Connect(+[](uint64_t letId, std::__thread_id id){
        std::stringstream ss; ss << "UDP Server binded and waiting for data on thread: " << id;
        log(xlet::UDPlet::letIdToString(letId) , ss.str());
    });

    udpServer.enableQueueManagement();

    auto qThread = std::function<void()>([&udpServer](){
        while(true) {

            if (udpServer.qin_.size() > 0) {

                //If Check peer is in nonHostingPeers.
                auto peerIdData = udpServer.qin_.front();
                udpServer.qin_.pop();

                //Get peerId
                auto peerId = peerIdData.first;

                //Check if it's a new connection. If so, add it to the nonHostingPeers set.
                if (nonHostingPeers.find(peerId) == nonHostingPeers.end()) newConnection(peerId);

                //Broadcast to all connected clients
                if (sHostId == peerId) for (auto& client : nonHostingPeers)  udpServer.pushData(client, peerIdData.second);
                //Route data to host.
                else udpServer.pushData(sHostId, peerIdData.second);
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
    //Argv[1] is the port
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    runServerRouting("0.0.0.0", port);

    return 0;

}
