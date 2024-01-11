#ifndef XLET_H
#define XLET_H

#include <mutex>
#include <queue>
#include <memory>
#include <thread>
#include <vector>
#include <cstddef>
#include <functional>

/* POSIX */
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* WINDOWS SHIT */

#define XLET_MAXBLOCKSIZE_SHIFTER   13
#define XLET_MAXBLOCKSIZE           (1 << XLET_MAXBLOCKSIZE_SHIFTER)

// Forward declaration of a template class Queue
namespace xlet {

    class Queue {

     public:
        void push(const std::vector<std::byte> dataOutBlock);
        bool pop(std::vector<std::byte>& dataInBlock);

     private:
        std::queue<std::vector<std::byte>> queue_;
        std::mutex mutex_;
    };


    enum Transport {
        UVGRTP, // UVG RTP
        UDS, //SOQ_SEQPACKET
        TCP, //SOCK_STREAM
        UDP  //SOCK_DGRAM
    };
    enum Direction {
        INB,
        OUTB,
        INOUTB
    };


    class Xlet {

     public:

        // Virtual destructor for proper cleanup in derived classes
        Xlet(){};
        virtual ~Xlet() = default;
        virtual bool valid () const {return false;}

        /*!
         *
         * @param data
         * @return
         */
        virtual std::size_t pushData(const std::vector<std::byte>& data) = 0;
        void setIncomingDataCallBack(std::function<void(std::vector<std::byte>&)> callback);


     protected:
        Transport transport;
        Direction direction;

        std::function<void(std::vector<std::byte>&)> incomingDataCallBack{[](auto& bytes){}};

        std::vector<struct pollfd> pollfds_;


    };

    class In  {
     protected:
        Queue qin_;

    };

    class Out  {
     protected:
        Queue qout_;
    };

    class InOut  {
     protected:
        Queue qin_;
        Queue qout_;
    };

#include "uds.h"
#include "udp.h"

    struct Configuration
    {
        Transport   transport{Transport::UDS};
        Direction   direction{Direction::INOUTB};
        std::string     address{"/tmp/tmpUDSserver"};
        std::string&    sockpath{address};
        int             port0{0};
        int             port1{0};
        int&            port{port0};
        int&            srcport{port0};
        int&            dstport{port1};

        bool            isValidConfiguration() const;

    };

    class Factory {
     public:

        static std::shared_ptr<Xlet> createXlet(const Configuration configuration);
    };

}





#endif // XLET_H

