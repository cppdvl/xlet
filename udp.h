#ifndef __UDPSERVER_H__
#define __UDPSERVER_H__


#include "xlet.h"

class UDPlet : public xlet::Xlet {
 protected:
    struct sockaddr_in  servaddr_;
    uint64_t            servId_;
    int                 sockfd_;
    bool                queueManaged{false};
 public:
    UDPlet(const std::string address, int port, xlet::Direction direction = xlet::Direction::INOUTB, bool theLetListens = false);
    ~UDPlet() override {}

    bool valid() const override {return sockfd_ >= 0;}
    std::size_t pushData(const uint64_t destId, const std::vector<std::byte>& data) override;
    std::size_t pushData(const std::vector<std::byte>& data) override;

    DAWn::Events::Signal<uint64_t>                                      letThreadStarted;
    DAWn::Events::Signal<const std::string, std::vector<std::byte>&>    letDataReadyToBeTransmitted;
    DAWn::Events::Signal<xlet::Data>                                    letDataFromServiceIsReadyToBeRead;
    DAWn::Events::Signal<uint64_t, std::vector<std::byte>&>&            letDataFromPeerReady{letDataFromConnectionIsReadyToBeRead};
    DAWn::Events::Signal<uint64_t, std::thread::id>                    letBindedOn;

    //Use only if needed
    std::thread qThread;
    std::thread recvThread;
    bool        qPause = true;
    std::thread inboundDataHandlerThread;

    inline void run()
    {
        qPause = false;
    }
    virtual void join() = 0;



    //Queue managed inbound Data
    void enableQueueManagement();
    void disableQueueManagement();
    //Static Helpers
    static struct sockaddr_in toSystemSockAddr(std::string ipstring, int port);
    static uint64_t sockAddrPortToUInt64(struct sockaddr_in& addr);
    static uint64_t sockAddToPeerId(struct sockaddr_in& sockAddr);
    static uint64_t sockAddrIpToUInt64(struct sockaddr_in& addr);
    static struct sockaddr_in peerIdToSockAddr(uint64_t peerId);
    static std::string letIdToIpString(uint64_t peerId);
    static std::string letIdToString(uint64_t peerId);
    static int peerIdToPort(uint64_t peerId);

};




class UDPOut : public UDPlet, public xlet::Out {

 public:
    UDPOut(const std::string address, int port, bool qSynced = false);
    ~UDPOut() override { close (sockfd_);}
    virtual void join() override;
};

class UDPIn : public UDPlet, public xlet::In {

 public:
    UDPIn(const std::string address, int port, bool qSynced = false);
    ~UDPIn() override {close (sockfd_);}
    virtual void join() override;

};

class UDPInOut : public UDPlet, public xlet::InOut {
 public:
    UDPInOut(const std::string address, int port, bool listen = false, bool qSynced = false);
    ~UDPInOut() override {close (sockfd_);}
    virtual void join() override;

};



#endif // __UDPSERVER_H__
