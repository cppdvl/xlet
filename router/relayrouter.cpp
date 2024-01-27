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


//TODO:UGLY HACK Change this.
using RTABLE = std::array<std::set<uint64_t>, 2>;

static std::map<uint64_t, std::weak_ptr<xlet::UDPInOut>> studios_router{};
static std::map<uint32_t, RTABLE> studios_routing{};
static std::map<uint64_t, std::map<uint32_t, uint64_t>> studios_uidtopeerid{};
xlet::UDPInOut* CreateRouter(std::string, int, bool, bool);
uint64_t CreateStudio(std::string ip, int port)
{
    auto sckadd             = xlet::UDPlet::toSystemSockAddr(ip,port);
    uint64_t studioIndex    = xlet::UDPlet::sockAddToPeerId(sckadd);

    auto studioFound = studios_router.find(studioIndex) != studios_router.end();
    if (studioFound)
    {
        auto sp = studios_router[studioIndex].lock();
        if (sp)
        {
            if (sp->valid()) return studioIndex;
            studios_router.erase(studioIndex);
        }
    }
    auto router = CreateRouter(ip, port, true, false);
    if (!router)
    {
        return 0;
    }

    auto pRouter = CreateRouter(ip, port, true, false);
    std::weak_ptr<xlet::UDPInOut> wpRouter = std::shared_ptr<xlet::UDPInOut>(pRouter);

    studios_router[studioIndex] = wpRouter;
    studios_routing[studioIndex] = RTABLE{};
    studios_uidtopeerid[studioIndex] = std::map<uint32_t, uint64_t>{};
    return studioIndex;
}

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

            auto serverId = ptrRouter->getServId();
            auto routingAvailable = studios_routing.find(serverId) != studios_routing.end();
            auto studioAvailable = studios_router.find(serverId) != studios_router.end();
            auto uid2peerAvailable = studios_uidtopeerid.find(serverId) != studios_uidtopeerid.end();


            //Check studio is indexed
            if (!routingAvailable || !studioAvailable || !uid2peerAvailable)
            {
                std::cout << "Routing (" << routingAvailable << ") or Studio (" << studioAvailable << ") or uid2peer(" << uid2peerAvailable <<") not available" << std::endl;
                return;
            }

            auto& routing = studios_routing[serverId];
            auto& uid2peer = studios_uidtopeerid[serverId];
            uint32_t uid = *(reinterpret_cast<uint32_t*>(data.data()));

            //Update Routing Table
            auto& routeFromSet = routing[uid%2];
            auto& routeToSet = routing[(uid+1)%2];
            auto notFound = routeFromSet.find(peerId) == routeFromSet.end();

            if (notFound)
            {
                if (!(uid%2)){

                    //This is group 0, eliminate all uids
                    for (auto&routeFromUID : routeFromSet)
                        uid2peer.erase(routeFromUID);

                    routeFromSet.clear();

                }
                routeFromSet.insert(uid);
            }

            if (uid2peer.find(uid) == uid2peer.end())
            {
                uid2peer[uid] = peerId;
            }

            //Get the TO group.
            for (auto& routeToUID : routeToSet)
            {
                auto& toPeerID = uid2peer[routeToUID];
                ptrRouter->qout_.push(std::make_pair(toPeerID, data));
            }

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





