#ifndef WEBSOCKETCLIENT_HPP
#define WEBSOCKETCLIENT_HPP
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using namespace std;

// Sends a WebSocket message and prints the response
class WebSocketClient : public enable_shared_from_this<WebSocketClient>
{
    private:
        tcp::resolver resolver_;
        websocket::stream<beast::tcp_stream> ws_;
        beast::flat_buffer buffer_;
        string host_;
        string text_;
        void fail(beast::error_code, char const*);

    public:
        explicit WebSocketClient(net::io_context&);
        void run(char const*, char const*, char const*);
        void on_resolve(beast::error_code, tcp::resolver::results_type);
        void on_connect(beast::error_code, tcp::resolver::results_type::endpoint_type);
        void on_handshake(beast::error_code);
        void on_write(beast::error_code, size_t);
        void on_read(beast::error_code, size_t);
        void on_close(beast::error_code);
};

#endif
