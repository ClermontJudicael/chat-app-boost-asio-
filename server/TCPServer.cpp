#include "TCPServer.hpp"

void tcp_server::handle_accept(std::shared_ptr<tcp_session> new_session,
                        const boost::system::error_code& error) {
    if (!error) {
        // connection accepted
        std::cout << "Client connected from: " << new_session->socket().remote_endpoint() << std::endl;
        // add new session to the list of active sessions
        active_sessions_->push_back(new_session);
        std::cout << "Current active sessions: " << active_sessions_->size() << std::endl;
        // start the newly accepted communication to begin communication
        new_session->start();
    } else {
        // an error during accept
        std::cerr << "Eccept error: " << error.message() << std::endl;
    }

    // keep listening for the next conection by startng another accept operation
    start_accept();
}

void tcp_server::start_accept()  {
    std::shared_ptr<tcp_session> new_session(
        new tcp_session(io_context_,
                        active_sessions_,
                        std::bind(&tcp_server::handle_session_disconnect, this, std::placeholders::_1))
    );

    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&tcp_server::handle_accept,
                                        this,
                                        new_session,
                                        boost::asio::placeholders::error));
                                        
    std::cout << "Server listening on port " << PORT << ". Waiting for new connections..." << std::endl;
}

void tcp_server::handle_session_disconnect(std::shared_ptr<tcp_session> session_to_remove) {
    std::cout << "Removing session for client: " << session_to_remove->socket().remote_endpoint() << std::endl;

    active_sessions_->remove(session_to_remove);
    std::cout << "Current active sessions: " << active_sessions_->size() << std::endl;
}
/////////////////////////////////tcp_session/////
void tcp_session::start() {
    socket_.async_read_some(
            boost::asio::buffer(read_buffer_, BUFFER_SIZE),
            boost::bind(&tcp_session::handle_read,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)
    );

    std::cout << "Session started for " << socket_.remote_endpoint() << ". Waiting foro data...\n";
}

void tcp_session::handle_read(const boost::system::error_code& error, size_t bytes_transfered) {
    if (!error) {
        std::string received_msg(read_buffer_, bytes_transfered);
        std::cout << "Reiceived " << bytes_transfered << "bytes from "
                  << socket_.remote_endpoint() << ": \"" << received_msg << "\"" << std::endl;

        // prepare msg for broadcast
        std::string broadcast_msg = "[" + socket_.remote_endpoint().address().to_string() + ":"
                                + std::to_string(socket_.remote_endpoint().port()) + "] says: "
                                + received_msg;
        
        // broadcast the msg to all active sessions
        // for some reason the compiler complain when i try to compare std::shared_ptr to boost::shared_ptr
        // dear GOD i thought they were the same fucking thing for fuck sake
        // i wont use boost::shared_ptr again
        for (const auto& session_ptr : *all_sessions_) {
            // ensure it's not the sender itself
            if (session_ptr && session_ptr != shared_from_this()) {
                std::shared_ptr<std::string> msg_to_send(new std::string(broadcast_msg));

                boost::asio::async_write(
                    session_ptr->socket(), // use other session's socket
                    boost::asio::buffer(*msg_to_send), // send the prepared msg
                    boost::bind(&tcp_session::handle_broadcast_write,
                                session_ptr, // bind to the specific session we are writting to
                                msg_to_send, // pass the shared_ptr to keep the msg alive
                                boost::asio::placeholders::error)
                );
            }
        }

        // initiate another rad operation after processiing the received msg
        socket_.async_read_some(
            boost::asio::buffer(read_buffer_, BUFFER_SIZE),
            boost::bind(&tcp_session::handle_read,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)
        );
    } else if (error == boost::asio::error::eof || 
               error == boost::asio::error::connection_reset ||
               error == boost::asio::error::bad_descriptor) {
        // client disconnecteed gracefully (EOF) or connection was reset unexpectedly
        // or the socket descriptor became invalid
        std::cout << "Client disconnected: " 
                  << socket_.remote_endpoint() 
                  << ". Error: " 
                  << error.message() 
                  << std::endl;
        // notify the server to remove this session from its active list
        session_completion_handler_(shared_from_this());
    } else {
        std::cerr << "Read error for " << socket_.remote_endpoint() << ": " << error.message() << std::endl;
        // also remove the session in case of other errors
        session_completion_handler_(shared_from_this());
    }
    
}

void tcp_session::handle_broadcast_write(std::shared_ptr<std::string> msg_to_send, const boost::system::error_code& error) {
    if (error) {
        std::cerr << "Broadcast write error to " << socket_.remote_endpoint() << error.message() << std::endl;
    }
}
