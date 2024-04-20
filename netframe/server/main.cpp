#include "EchoSocketMgr.h"

#include <boost/asio/io_context.hpp>
#include <iostream>
#include <memory>

int main()
{
    std::shared_ptr<boost::asio::io_context> ioContext = std::make_shared<boost::asio::io_context>();

    if (!sEchoSocketMgr.StartNetwork(*ioContext, "0.0.0.0", 8989, 1))
    {
        std::cout << "Failed to initialize network" << std::endl;
        return 1;
    }

    ioContext->run();

    sEchoSocketMgr.StopNetwork();
    return 0;
}