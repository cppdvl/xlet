//
// Created by Julian Guarin on 16/01/24.
//

#include "xlet.h"

static std::set<uint64_t>   authorizedPeers{};
static std::set<uint64_t>   peers{};
static uint64_t             hostingPeerId{0};
static bool                 requiredAuth{true};
DAWn::Events::Signal<uint64_t>  nonAuthorizedPeerAttemptedAConnection;
static std::set<uint64_t>   attemptedPeers{};

bool isAuthorizedPeer(uint64_t peerId)
{
    return (!requiredAuth) || (authorizedPeers.find(peerId) != authorizedPeers.end());
}


xlet::UDPInOut* CreateRouter (std::string ip, int port, bool listen, bool synced)
{
    auto ptrRouter = new xlet::UDPInOut(ip, port, listen, synced);
    if (!ptrRouter) return nullptr;
    if (ptrRouter->valid() == false)
    {
        delete ptrRouter;
        return nullptr;
    }
    ptrRouter->letOperationalError.Connect(std::function<void(uint64_t, std::string)>{[](uint64_t peerId, std::string error) {
        std::cout << peerId << " " << error << std::endl;
    }});
    ptrRouter->letBindedOn.Connect(std::function<void(uint64_t, std::thread::id)>{[](uint64_t peerId, std::thread::id threadId) {
        std::cout << peerId << " " << threadId << std::endl;
    }});
    ptrRouter->letThreadStarted.Connect(std::function<void(uint64_t)>{[](uint64_t peerId) {
        std::cout << peerId << " Thread Started" << std::endl;
    }});

    nonAuthorizedPeerAttemptedAConnection.Connect(std::function<void(uint64_t)>{[](uint64_t peerId) {
        if (attemptedPeers.find(peerId) == attemptedPeers.end()) {
            std::cout << peerId << " Non Authorized Peer Attempted A Connection" << std::endl;
            attemptedPeers.insert(peerId);
        }
    }});

    ptrRouter->letDataFromPeerIsReady.Connect(
    std::function<void(uint64_t, std::vector<std::byte>&)>{
        [ptrRouter](uint64_t peerId, std::vector<std::byte>& data){
            if (!isAuthorizedPeer(peerId)) {
                nonAuthorizedPeerAttemptedAConnection.Emit(peerId);
                return;
            }
            //The Peer is authorized. Forward data.
            if (!hostingPeerId) hostingPeerId = peerId;
            else peers.insert(peerId);

            for (auto&peer : peerId == hostingPeerId ? peers : std::set<uint64_t>{hostingPeerId})
                ptrRouter->qout_.push(std::make_pair(peer, data));
        }
    });
    return ptrRouter;
}


void disableAuthRequirement()
{
    requiredAuth = false;
}

void enableAuthRequirement()
{
    requiredAuth = true;
}

void addAuthorizedPeer(uint64_t peerId)
{
    authorizedPeers.insert(peerId);
}

void removeAuthorizedPeer(uint64_t peerId)
{
    authorizedPeers.erase(peerId);
}



