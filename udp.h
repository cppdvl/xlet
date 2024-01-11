#ifndef __UDPSERVER_H__
#define __UDPSERVER_H__


#include "xlet.h"

class UDPlet : public xlet::Xlet {
    protected:
        struct sockaddr_in  servaddr_;
        int sockfd_;

    public:

    UDPlet(const std::string address, int port, xlet::Direction direction = xlet::Direction::INOUTB, bool theLetListens = false);
    ~UDPlet() override {}

    bool valid() const override {return sockfd_ < 0;}
    std::size_t pushData(const std::vector<std::byte>& data) override;
    std::function<void()> inboundDataHandler = []()->void{};

};




class UDPOut : public UDPlet, public xlet::Out {

 public:
    UDPOut(const std::string address, int port);
    ~UDPOut() override {}
};

class UDPIn : public UDPlet, public xlet::In {

 public:
    UDPIn(const std::string address, int port);
    ~UDPIn() override {}

};

class UDPInOut : public UDPlet, public xlet::In {
public:
    UDPInOut(const std::string address, int port, bool listen = false);
    ~UDPInOut() override {}

};



#endif // __UDPSERVER_H__
