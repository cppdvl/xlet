#include "xlet.h"
#include <arpa/inet.h>


struct sockaddr_in toSystemSockAddr(std::string ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    //inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    return addr;
}




xlet::UDPlet::UDPlet(const std::string ipstring, int port, xlet::Direction direction, bool theLetListens) /*: Xlet()*/
{

    servaddr_ = toSystemSockAddr(ipstring, port);

    /* If Direction is In Only, then theLetListens must be true */
    if ( direction == xlet::Direction::INB && theLetListens == false)
    {
        //TODO: Trigger a warning signal
        theLetListens = true;
    }
    else if ( direction == xlet::Direction::OUTB && theLetListens == true)
    {
        //TODO: Trigger a warning signal
        theLetListens = false;
    }

    if ((sockfd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        //TODO: Trigger a critical error signal
        sockfd_ = -1;
    }
    else if (theLetListens)
    {
        if (bind(sockfd_, (struct sockaddr *)&servaddr_, sizeof(servaddr_)) < 0)
        {
            //TODO: Trigger a critical error signal
            close(sockfd_);
            sockfd_ = -1;
            return;
        }
        inboundDataHandler = std::function<void()>{[this](){

            struct sockaddr_in cliaddr;
            socklen_t len = sizeof(cliaddr);
            while(true) {
                std::vector<std::byte> inDataBuffer(XLET_MAXBLOCKSIZE, std::byte{0});
                int bytesReceived = 0;
                if ((bytesReceived = recvfrom(sockfd_, inDataBuffer.data(), inDataBuffer.size(), 0, (struct sockaddr *) &cliaddr, &len)) < 0) {
                    //TODO: Triger error signal
                    continue;
                }
                inDataBuffer.resize(bytesReceived);
            }
        }};

    }
    //TODO: Trigger a signal that the let is socket.
}

std::size_t xlet::UDPlet::pushData(const std::vector<std::byte>& data) {
    if (sockfd_ < 0) {
        //TODO: Trigger a critical error signal
        return 0;
    }
    auto bytesSent = static_cast<size_t>(0);
    while (bytesSent < data.size()) {
        auto bytesSentNow = sendto(sockfd_, data.data() + bytesSent, data.size() - bytesSent, 0, (struct sockaddr *) &servaddr_, sizeof(servaddr_));
        if (bytesSentNow < 0) {
            //Trigger a critical error signal
            return 0;
        }
        bytesSent += bytesSentNow;
    }
    if (bytesSent != data.size()) {
        //Trigger a Critical Warning signal.
    }
    return bytesSent;
}
/********************/
/****** UDPOut ******/
xlet::UDPOut::UDPOut(const std::string ipstring, int port) : UDPlet(ipstring, port, xlet::Direction::OUTB, false)
{
}
/********************/
/******** UDPIn ******/
xlet::UDPIn::UDPIn(const std::string ipstring, int port) : UDPlet(ipstring, port, xlet::Direction::INB, true)
{
}
/********************/
/****** UDPInOut *****/
xlet::UDPInOut::UDPInOut(const std::string ipstring, int port, bool listen) : UDPlet(ipstring, port, xlet::Direction::INOUTB, listen)
{
}


