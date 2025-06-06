#include <boost/bind/bind.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <memory>

#define PORT 12345
#define BUFFER_SIZE 1024

using boost::asio::ip::tcp;

class tcp_session : public std::enable_shared_from_this<tcp_session> {
public:
    tcp_session(boost::asio::io_context& io_context) : socket_(io_context) {}

    tcp::socket& socket() { return socket_; }

    void start();

private:

    void handle_read(const boost::system::error_code& error, size_t bytes_transfered);

    void handle_write(const boost::system::error_code& error);

    tcp::socket socket_; // for client connection
    char data_[BUFFER_SIZE];
};

class tcp_server {
public:
    tcp_server(boost::asio::io_context& io_context, short port)
        : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }
private:
    void start_accept();

    void handle_accept(std::shared_ptr<tcp_session> new_session,
                        const boost::system::error_code& ec);

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_; // the acceptoro for incomming connections
};