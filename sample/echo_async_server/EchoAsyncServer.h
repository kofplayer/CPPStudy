//
// Created by Administrator on 2023/8/18.
//

#ifndef CPPSTUDY_ECHOASYNCSERVER_H
#define CPPSTUDY_ECHOASYNCSERVER_H

#include <boost/asio.hpp>
#include <string>
#include <unordered_map>

class EchoAsyncServer {
public:
    EchoAsyncServer();
    struct ConnectInfo {
        uint64_t id;
        std::shared_ptr<boost::asio::ip::tcp::socket> socket;
        std::array<char, 1024> readBuffer;
        std::list<std::shared_ptr<std::string>> sendMsgs;
    };
    bool start(const boost::asio::ip::tcp::endpoint& endpoint);
protected:
    void acceptHandler(const boost::system::error_code &ec);
    void readHandler(const boost::system::error_code& ec, std::size_t bytes_transferred, uint64_t socketId);
    void writeHandler(const boost::system::error_code &ec, std::size_t bytes_transferred, uint64_t socketId);
    boost::asio::io_service m_ioService;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    std::shared_ptr<boost::asio::ip::tcp::endpoint> m_endpoint;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
    std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    std::unordered_map<uint64_t, std::shared_ptr<ConnectInfo>> m_connectInfos;

    uint64_t m_genId;
};


#endif //CPPSTUDY_ECHOASYNCSERVER_H
