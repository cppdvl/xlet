//
// Created by Julian Guarin on 16/01/24.
//

#ifndef XLETLIB_RELAYROUTER_H
#define XLETLIB_RELAYROUTER_H

#include "xlet.h"
#include <set>
#include <thread>
#include <functional>


namespace DAWn::Relay
{
    void routerThread(int argc, char**argv);
    void restAPIThread(int argc, char**argv);
}
#endif //XLETLIB_RELAYROUTER_H
