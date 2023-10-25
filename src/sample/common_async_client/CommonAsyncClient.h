//
// Created by Administrator on 2023/9/16.
//

#ifndef CPPSTUDY_COMMONASYNCCLIENT_H
#define CPPSTUDY_COMMONASYNCCLIENT_H

#include <functional>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

class CommonAsyncClient {
public:
    enum class Status {
        None = 0,
        Connecting = 1,
        Connected = 2
    };
    enum class ErrorCode {
        Success = 0,
        ConnectError = 1,
        ReadError = 2,
        WriteError
    };

    using OnConnectedCallBack = std::function<void(ErrorCode)>;
    using OnDisconnectedCallBack = std::function<void()>;
    using OnMessageCallBack = std::function<void(const char* data, size_t len)>;

    bool connect(const boost::asio::ip::tcp::endpoint& endpoint,
                 const std::shared_ptr<boost::asio::io_service>& ioService);
    void onConnect(OnConnectedCallBack& callback);
    void onDisconnect(OnDisconnectedCallBack& callback);
    void onMessage(OnMessageCallBack& callback);
    bool sendMessage(const char* data, size_t len);
    bool close();
    Status getStatus();
protected:
    void readHandler(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void writeHandler(const boost::system::error_code &ec, std::size_t bytes_transferred, std::shared_ptr<std::string> data);
    void onError(ErrorCode ec);

    Status m_status;
    std::array<char, 1024> m_receiveBuffer;
    std::list<std::shared_ptr<std::string>> m_sendMessages;
    std::shared_ptr<boost::asio::io_service> m_ioService;
    std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    OnConnectedCallBack m_onConnect;
    OnDisconnectedCallBack m_onDisconnect;
    OnMessageCallBack m_onMessage;
};


#endif //CPPSTUDY_COMMONASYNCCLIENT_H
