#include "server/TCPServer.hpp"

int main(int argc, char* argv[]) {
    try {
        boost::asio::io_context io_context;
        tcp_server server(io_context, PORT);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}