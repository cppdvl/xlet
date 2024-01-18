//
// Created by Julian Guarin on 16/01/24.
//

#include "relayrouter.h"

static std::set<uint64_t>   authorizedPeers{};
static std::set<uint64_t>   peers{};
static uint64_t             hostingPeerId{0};
static bool                 nonRequiredAuth{false};
DAWn::Events::Signal<uint64_t>  nonAuthorizedPeerAttemptedAConnection;
static std::set<uint64_t>   attemptedPeers{};
bool isAuthorizedPeer(uint64_t peerId)
{

    return nonRequiredAuth || (authorizedPeers.find(peerId) != authorizedPeers.end());
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

    ptrRouter->letDataFromConnectionIsReadyToBeRead.Connect(
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

