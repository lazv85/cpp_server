#include <iostream>
#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

using namespace std;
using namespace boost;
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;


class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        do_read();
    }

private:
    void do_read() {
        std::cout << "doing do_read" << std::endl;

        http::async_read(socket_, buffer_, request_,
            [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {                
                std::cout << "doing async_read: " << ec << " " << request_ << " "  << std::endl;     
                if (!ec) {
                    handle_request();
                }
            });
    }

    void handle_request() {
        std::cout << "connection accepted" << std::endl;
        response_.version(request_.version());
        response_.keep_alive(false);
        response_.result(http::status::ok);
        response_.set(http::field::server, "MyServer");
        response_.set(http::field::content_type, "text/html");
        response_.body() = "Hello, World!";
        response_.prepare_payload();
        do_write();
    }

    void do_write() {
        std::cout << "doing do_write" << std::endl;

        http::async_write(socket_, response_,
            [this](const boost::system::error_code& ec, std::size_t) {
                std::cout << "doing async_write" << std::endl;

                boost::system::error_code ecc;
                socket_.shutdown(tcp::socket::shutdown_send, ecc);
            });
    }

    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> request_;
    http::response<http::string_body> response_;
};

class WebServer {
public:
    WebServer(asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          socket_(io_context) {
        do_accept();
    }

private:
    void do_accept() {
        std::cout << "doing do_accept" << std::endl;

        acceptor_.async_accept(socket_,
                               [this](const boost::system::error_code& ec) {
                                    std::cout << "doing async_accept" << std::endl;

                                   if (!ec) {
                                       std::make_shared<session>(std::move(socket_))->start();
                                   }
                                   do_accept();
                               });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

int main() {
    try {
        std::cout << "starting server" << std::endl;
        asio::io_context io_context;
        WebServer server(io_context, 8080);
        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
