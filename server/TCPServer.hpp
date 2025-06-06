#include <boost/bind/bind.hpp>
#include <functional>
#include <iostream>
#include <boost/asio.hpp>
#include <list>
#include <memory>

#define PORT 12345
#define BUFFER_SIZE 1024

// forward declaration
class tcp_session;

// define a type for the list of active sessions, shared among sessions
typedef std::shared_ptr<std::list<std::shared_ptr<tcp_session>>> shared_session_list;

using boost::asio::ip::tcp;

class tcp_session : public std::enable_shared_from_this<tcp_session> {
public:
    tcp_session(boost::asio::io_context& io_context,
                shared_session_list all_sessions,
                std::function<void(std::shared_ptr<tcp_session>)> session_completion_handler)
        : socket_(io_context),
          all_sessions_(all_sessions),
          session_completion_handler_(session_completion_handler) {}

    tcp::socket& socket() { return socket_; }

    void start();

private:

    void handle_read(const boost::system::error_code& error, size_t bytes_transfered);

    void handle_broadcast_write(std::shared_ptr<std::string> msg_to_send, const boost::system::error_code& error);

    tcp::socket socket_; // for client connection
    char read_buffer_[BUFFER_SIZE];
    shared_session_list all_sessions_;
    // callback to notify server on disconnect
    std::function<void(std::shared_ptr<tcp_session>)> session_completion_handler_;
};

class tcp_server {
public:
    tcp_server(boost::asio::io_context& io_context, short port)
        : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          active_sessions_(new std::list<std::shared_ptr<tcp_session>>()) // initialize the shared list of active sessions
    {
        start_accept();
    }
private:
    void start_accept();
    void handle_accept(std::shared_ptr<tcp_session> new_session,
                       const boost::system::error_code& error);
    void handle_session_disconnect(std::shared_ptr<tcp_session> session_to_remove);

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_; // the accept for for incomming connections
    shared_session_list active_sessions_;
};