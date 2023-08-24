//
// Created by Administrator on 2023/8/22.
//

#ifndef CPPSTUDY_COMMONASYNCSERVER_H
#define CPPSTUDY_COMMONASYNCSERVER_H


#include <functional>
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

class CommonAsyncServer {
public:
    using connect_id_type = uint64_t;

    CommonAsyncServer();
    bool start(const boost::asio::ip::tcp::endpoint& endpoint, int rwThreadCount);
    bool stop();

    void onConnect(const std::function<void(connect_id_type, const std::string &)>& callback);
    void onDisconnect(const std::function<void(connect_id_type)>& callback);
    void onMessage(const std::function<void(connect_id_type, const char*, size_t)>& callback);

    bool sendMessage(connect_id_type connectId, const char* data, size_t len);
    bool close(connect_id_type connectId);
protected:
    struct ConnectInfo {
        connect_id_type connectId;
        std::shared_ptr<boost::asio::ip::tcp::socket> socket;
        std::array<char, 1024> readBuffer;
        std::list<std::shared_ptr<std::string>> sendMsgs;
    };
    struct Worker {
        boost::asio::io_service ioService;
        std::shared_ptr<boost::asio::io_service::work> work;
        std::unordered_map<connect_id_type, std::shared_ptr<ConnectInfo>> connectInfos;
    };
    void acceptHandler(const boost::system::error_code &ec);
    void readHandler(const boost::system::error_code& ec, std::size_t bytes_transferred, connect_id_type connectId, const std::shared_ptr<Worker>& worker);
    void writeHandler(const boost::system::error_code &ec, std::size_t bytes_transferred, connect_id_type connectId, const std::shared_ptr<Worker>& worker);

    std::shared_ptr<Worker> getWorkerByConnectId(connect_id_type connectId);
    boost::asio::io_service m_ioService;
    std::shared_ptr<boost::asio::io_service::work> m_work;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
    std::vector<std::shared_ptr<Worker>> m_workers;
    boost::asio::detail::thread_group m_threadGroup;

    std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    std::shared_ptr<Worker> m_genWorker;
    connect_id_type m_genConnectId;

    std::function<void(connect_id_type, const std::string & address)> m_onConnect;
    std::function<void(connect_id_type)> m_onDisconnect;
    std::function<void(connect_id_type, const char* data, size_t len)> m_onMessage;

    bool m_running;
};


#endif //CPPSTUDY_COMMONASYNCSERVER_H
