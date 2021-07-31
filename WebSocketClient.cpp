#include "WebSocketClient.hpp"

// Resolver and socket require an io_context
WebSocketClient::WebSocketClient(net::io_context& ioc) : resolver_(net::make_strand(ioc)), ws_(net::make_strand(ioc)) {}

// Start the asynchronous operation
void WebSocketClient::run(char const* host, char const* port, char const* text)
{
    // Save these for later
    host_ = host;
    text_ = text;

    // Look up the domain name
    resolver_.async_resolve(host, port, beast::bind_front_handler(&WebSocketClient::on_resolve, shared_from_this()));
}

void WebSocketClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if(ec)
        return fail(ec, "resolve");

    // Set the timeout for the operation
    beast::get_lowest_layer(ws_).expires_after(chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(results, beast::bind_front_handler(&WebSocketClient::on_connect, shared_from_this()));
}

void WebSocketClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
    if(ec)
        return fail(ec, "connect");

    // Turn off the timeout on the tcp_stream, because the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::user_agent, string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
        })
    );

    // Update the host_ string. 
    // This will provide the value of the host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    host_ += ':' + to_string(ep.port());

    // Perform the websocket handshake
    ws_.async_handshake(host_, "/", beast::bind_front_handler(&WebSocketClient::on_handshake, shared_from_this()));
}

void WebSocketClient::on_handshake(beast::error_code ec)
{
    if(ec)
        return fail(ec, "handshake");

    cout << "Enter row col: ";
    getline(cin, text_);
        
    // Send the message
    ws_.async_write(net::buffer(text_), beast::bind_front_handler(&WebSocketClient::on_write, shared_from_this()));
}

void WebSocketClient::on_write(beast::error_code ec, size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    
    if(ec)
        return fail(ec, "write");
        
    // Read a message into our buffer
    ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketClient::on_read, shared_from_this()));
}

void WebSocketClient::on_read(beast::error_code ec, size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    
    if(ec)
        return fail(ec, "read");

    string receivedString = beast::buffers_to_string(buffer_.data());
    cout << receivedString << endl;
    buffer_.clear();

    if (receivedString == "won" || receivedString == "tie" || receivedString == "lose")
    {
        cout << "won or tie or lose" << endl;
        // Close the WebSocket connection
        ws_.async_close(websocket::close_code::normal, beast::bind_front_handler(&WebSocketClient::on_close, shared_from_this()));
    }
    else
    {
        cout << "Enter row col: ";
        getline(cin, text_);

        // Close the WebSocket connection
        // ws_.async_close(websocket::close_code::normal, beast::bind_front_handler(&WebSocketClient::on_close, shared_from_this()));

        // Send the message
        ws_.async_write(net::buffer(text_), beast::bind_front_handler(&WebSocketClient::on_write, shared_from_this()));
    }
}

void WebSocketClient::on_close(beast::error_code ec)
{
    if(ec)
        return fail(ec, "close");

    // If we get here then the connection is closed gracefully

    // The make_printable() function helps print a ConstBufferSequence
    cout << beast::make_printable(buffer_.data()) << endl;
}

// Report a failure
void WebSocketClient::fail(beast::error_code ec, char const* what)
{
    cerr << what << ": " << ec.message() << "\n";
}
