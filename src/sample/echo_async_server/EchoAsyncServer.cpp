//
// Created by Administrator on 2023/8/18.
//

#include "EchoAsyncServer.h"
#include <memory>
#include <iostream>

EchoAsyncServer::EchoAsyncServer() : m_genId(0) {

}

bool EchoAsyncServer::start(const boost::asio::ip::tcp::endpoint& endpoint) {
    m_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(m_ioService, endpoint);
    m_socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioService);
    m_work = std::make_shared<boost::asio::io_service::work>(m_ioService);
    m_acceptor->listen();
    m_acceptor->async_accept(*m_socket, [this](const boost::system::error_code &ec) { acceptHandler(ec); });
    m_ioService.run();
    return true;
}

void EchoAsyncServer::acceptHandler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        auto ci = new ConnectInfo();
        ci->socket = m_socket;
        ci->id = ++m_genId;
        std::cout << "new connect: " << ci->id << std::endl;
        m_connectInfos.insert(std::make_pair(ci->id, std::shared_ptr<ConnectInfo>(ci)));
        m_socket->async_read_some(boost::asio::buffer(ci->readBuffer), [this, connectId = ci->id] (const boost::system::error_code& ec, std::size_t bytes_transferred) { readHandler(ec, bytes_transferred, connectId); });

        m_socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioService);
        m_acceptor->async_accept(*m_socket, [this] (const boost::system::error_code &ec) { acceptHandler(ec); });
    }
}

void EchoAsyncServer::readHandler(const boost::system::error_code& ec, std::size_t bytes_transferred, uint64_t connectId) {
    if (!ec) {
        if (!m_connectInfos.contains(connectId)) {
            return;
        }
        std::shared_ptr<ConnectInfo> ci = m_connectInfos[connectId];
        std::string msg(ci->readBuffer.data(), bytes_transferred);
        std::cout << "receive id:" << connectId << " msg:" << msg << std::endl;
        ci->sendMsgs.push_back(std::make_shared<std::string>(msg));
        if (ci->sendMsgs.size() == 1) {
            ci->socket->async_send(boost::asio::buffer(*ci->sendMsgs.front()), [this, connectId ](const boost::system::error_code &ec, std::size_t bytes_transferred) { writeHandler(ec, bytes_transferred, connectId); });
        }
        ci->socket->async_read_some(boost::asio::buffer(ci->readBuffer), [this, connectId] (const boost::system::error_code& ec, std::size_t bytes_transferred) { readHandler(ec, bytes_transferred, connectId); });
    }
    else {
        // 断开连接
        std::cout << "disconnect1 id:" << connectId << std::endl;
        m_connectInfos.erase(connectId);
    }
}

void EchoAsyncServer::writeHandler(const boost::system::error_code &ec, std::size_t bytes_transferred, uint64_t connectId)
{
    if (!m_connectInfos.contains(connectId)) {
        return;
    }
    if (ec) {
        std::cout << "disconnect2 id:" << connectId << " " << ec.what() << std::endl;
        m_connectInfos.erase(connectId);
        return;
    }
    std::shared_ptr<ConnectInfo> ci = m_connectInfos[connectId];
    assert(!ci->sendMsgs.empty());
    if (ci->sendMsgs.empty()) {
        return;
    }
    assert(bytes_transferred == ci->sendMsgs.front()->size());
    ci->sendMsgs.pop_front();
    if (!ci->sendMsgs.empty()) {
        ci->socket->async_send(boost::asio::buffer(*ci->sendMsgs.front())
                               ,[this, connectId](const boost::system::error_code &ec, std::size_t bytes_transferred) { writeHandler(ec, bytes_transferred, connectId); });
    }
}

bool EchoAsyncServer::stop() {
    m_work.reset();
    m_ioService.stop();
    return true;
}
