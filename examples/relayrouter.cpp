//
// Created by Julian Guarin on 10/01/24.
//
#include "xlet.h"

xlet::UDPInOut* CreateRouter (int port);

int main(int argc, char**argv)
{
    auto router = CreateRouter(8899);
    if (router)
    {
        router->run();
        router->join();

    }
    delete router;
    return 0;

}
