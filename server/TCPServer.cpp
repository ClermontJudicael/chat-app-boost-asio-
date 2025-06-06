#include "TCPServer.hpp"

void tcp_server::handle_accept(std::shared_ptr<tcp_session> new_session,
                        const boost::system::error_code& ec) {
                             if (!ec) {
        std::cout << "client connected from: " << new_session->socket().remote_endpoint() << std::endl;
            new_session->start();
        } else {
            std::cerr << "accept error:  " << ec.message() << std::endl;
        }

        start_accept();
}

void tcp_server::start_accept()  {
    std::shared_ptr<tcp_session> new_session(new tcp_session(io_context_));
    // asynchronously accept a new connection
    // if accepted, handle_accept will be called
    acceptor_.async_accept(new_session->socket(),
                        boost::bind(&tcp_server::handle_accept,
                        this,
                        new_session,
                        boost::asio::placeholders::error));
    std::cout << "Server listening on port " << PORT << "waiting for connections...\n";
}

/////////////////////////////////tcp_session/////
void tcp_session::start() {
    socket_.async_read_some(
        boost::asio::buffer(data_, BUFFER_SIZE),
        boost::bind(&tcp_session::handle_read,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred())
    );

    std::cout << "Session hath commenced. Awaiting the arrival of data...\n";
}

void tcp_session::handle_read(const boost::system::error_code& error, size_t bytes_transfered) {
    if (!error) {
        // echo back to client
        std::cout << "received" << bytes_transfered << " bytes from client: "
                  << std::string(data_, bytes_transfered) << std::endl;
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(data_, bytes_transfered),
            boost::bind(&tcp_session::handle_write,
                           shared_from_this(),
                           boost::asio::placeholders::error())
        );
    } else {
        std::cerr << "read error: " << error.message() << std::endl;
    }
}

void tcp_session::handle_write(const boost::system::error_code& error) {
        if (!error) {
            // data is written
            std::cout << "data echoed back. GIVE ME MORE DATA TO READ, FOR THAT IS MY PURPOSE...." << std::endl;
            socket_.async_read_some(
                boost::asio::buffer(data_, BUFFER_SIZE),
                boost::bind(&tcp_session::handle_read,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred())
            );
        } else {
            std::cerr << "write error: " << error.message() << std::endl;
        }
}