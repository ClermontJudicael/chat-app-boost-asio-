#include "server/TCPServer.hpp"
#include <boost/asio/io_context.hpp>
#include <exception>
#include <iostream>

int main() {
    try {
        boost::asio::io_context io_context;
        tcp_server server(io_context, PORT);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}