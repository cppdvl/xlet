#include "xlet.h"
#include <arpa/inet.h>

namespace UDPUtilities
{
    struct sockaddr_in toSystemSockAddr(std::string ip, int port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        //inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());

        return addr;
    }

    uint64_t sockAddrIpToUInt64(struct sockaddr_in& addr)
    {
        uint64_t ip = *reinterpret_cast<uint32_t*>(&addr.sin_addr.s_addr);
        ip          <<= 32;
        return ip;
    }

    uint64_t sockAddrPortToUInt64(struct sockaddr_in& addr)
    {
        return static_cast<uint64_t>(ntohs(addr.sin_port));
    }

    uint64_t sockAddToPeerId(struct sockaddr_in& addr)
    {
        return sockAddrIpToUInt64(addr) | sockAddrPortToUInt64(addr);
    }

    struct sockaddr_in peerIdToSockAddr(uint64_t peerId)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint32_t>(peerId & 0xFFFF));
        addr.sin_addr.s_addr = htonl(peerId >> 32);
        return addr;
    }

}



xlet::UDPlet::UDPlet(
    const std::string ipstring,
    int port,
    xlet::Direction direction,
    bool theLetListens
)
{
    this->direction = direction;
    servaddr_       = UDPUtilities::toSystemSockAddr(ipstring, port);
    servId_         = UDPUtilities::sockAddToPeerId(servaddr_);

    if ( direction == xlet::Direction::INB && theLetListens == false)
    {
        theLetListens = true;
    }
    else if ( direction == xlet::Direction::OUTB && theLetListens == true)
    {
        theLetListens = false;
    }

    if ((sockfd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        sockfd_ = -1;
        return;
    }

    if (theLetListens)
    {
        if (bind(sockfd_, (struct sockaddr *)&servaddr_, sizeof(servaddr_)) < 0)
        {
            letOperationalError.Emit(sockfd_, "bind");
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
                    letOperationalError.Emit(sockfd_, "recvfrom");
                    continue;
                }

                inDataBuffer.resize(bytesReceived);
                letDataFromConnectionIsReadyToBeRead.Emit(UDPUtilities::sockAddToPeerId(cliaddr), inDataBuffer);
            }

        }};
    }
    else
    {
        inboundDataHandler = std::function<void()>{[this](){
            while(true) {
                std::vector<std::byte> inDataBuffer(XLET_MAXBLOCKSIZE, std::byte{0});
                int bytesReceived = 0;
                if ((bytesReceived = recvfrom(sockfd_, inDataBuffer.data(), inDataBuffer.size(), 0, nullptr, nullptr)) < 0) {
                    letOperationalError.Emit(sockfd_, "recvfrom");
                    continue;
                }

                inDataBuffer.resize(bytesReceived);
                letDataFromServiceIsReadyToBeRead.Emit(inDataBuffer);
            }

        }};
    }
}

std::size_t xlet::UDPlet::pushData(const std::vector<std::byte>& data) {
    return pushData(servId_, data);
}
std::size_t xlet::UDPlet::pushData(const uint64_t peerId,  const std::vector<std::byte>& data) {
    if (sockfd_ < 0) {
        //TODO: Trigger a critical error signal
        return 0;
    }

    auto addr = UDPUtilities::peerIdToSockAddr(peerId);
    auto bytesSent = static_cast<size_t>(0);
    while (bytesSent < data.size()) {
        auto bytesSentNow = sendto(sockfd_, data.data() + bytesSent, data.size() - bytesSent, 0, (struct sockaddr *) &addr, sizeof(addr));
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


