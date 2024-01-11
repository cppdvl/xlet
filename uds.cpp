#include "xlet.h"

#include <thread>
#include <sys/un.h>
#include <functional>

xlet::UDSlet::UDSlet(const std::string &sockpath, xlet::Direction direction, bool theLetListens)
{
    if ( direction == xlet::Direction::INB && theLetListens == false)
    {
        //TODO: Trigger a warning signal
        theLetListens = true;
    }

    if ((sockfd_ = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0)
    {
        //TODO: Trigger a warning signal
        sockfd_ = -1;
        return;
    }
    if (theLetListens)
    {
        struct sockaddr_un servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = AF_UNIX;
        strncpy(servaddr.sun_path, sockpath.c_str(), sizeof(servaddr.sun_path)-1);
        unlink(sockpath.c_str());

        if (bind(sockfd_, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        {
            //TODO: Trigger a critical error signal.
            close(sockfd_);
            sockfd_ = -1;
            return;
        }

        if (listen(sockfd_, 1) < 0)
        {
            //TODO: Trigger a critical error signal.
            close(sockfd_);
            return;
        }
        pollfds_.push_back({sockfd_, POLLIN, 0});
        inboundDataHandler = std::function<void()>{[this]() {
            while (true) {

                if(poll(pollfds_.data(), pollfds_.size(), -1) < 0){
                    //TODO: Trigger a critical error signal.
                    return 0;
                }

                for (auto&fd : pollfds_)
                {
                    if (fd.revents & POLLIN)
                    {
                        if (fd.fd == sockfd_)
                        {
                            int connfd = accept(sockfd_, nullptr, nullptr);
                            if (connfd < 0) {
                                //TODO: Trigger a warning error signal.
                                continue;
                            }
                            pollfds_.push_back({connfd, POLLIN, 0});
                        }
                        else
                        {
                            std::vector<std::byte> dataInBlock(XLET_MAXBLOCKSIZE, std::byte{0});
                            ssize_t bytesRead = read(fd.fd, dataInBlock.data(), dataInBlock.size());
                            if (bytesRead <= 0) {

                                //TODO: Trigger connection will be closed.
                                close (fd.fd);
                                fd.fd = -1;
                                continue;
                            }
                            dataInBlock.resize(bytesRead);
                            //TODO: Trigger a signal that data has been received.
                        }
                    }
                }
                pollfds_.erase(std::remove_if(pollfds_.begin(), pollfds_.end(), [](auto& fd){return fd.fd < 0;}), pollfds_.end());
            }
        }};
    }

}

std::size_t xlet::UDSlet::pushData(const std::vector<std::byte>& data) {

    if (sockfd_ < 0) {
        //TODO: Trigger a critical error signal
        return 0;
    }

    const std::byte *dataPtr = data.data();
    std::size_t dataLen = data.size();

    //write thru socket
    while (dataLen > 0) {
        ssize_t bytesWritten = write(sockfd_, dataPtr, dataLen);
        if (bytesWritten < 0) {
            //TODO: Trigger a critical error signal
            return 0;
        }
        dataPtr += bytesWritten;
        dataLen -= bytesWritten;
    }

    return data.size();
}

/********************/
/****** UDSOut ******/
xlet::UDSOut::UDSOut(const std::string &sockpath) : UDSlet(sockpath, xlet::Direction::OUTB, false)
{
}

/********************/
/****** UDSIn ******/
xlet::UDSIn::UDSIn(const std::string &sockpath) : UDSlet(sockpath, xlet::Direction::INB, true)
{
}

/********************/
/****** UDSInOut ****/
xlet::UDSInOut::UDSInOut(const std::string &sockpath, bool listen) : UDSlet(sockpath, xlet::Direction::INOUTB, listen)
{
}
