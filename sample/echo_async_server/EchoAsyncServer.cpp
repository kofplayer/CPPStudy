//
// Created by Administrator on 2023/8/18.
//

#include "EchoAsyncServer.h"
#include <boost/bind/bind.hpp>

#include <memory>
#include <iostream>

EchoAsyncServer::EchoAsyncServer() : m_genId(0) {

}

bool EchoAsyncServer::start(const boost::asio::ip::tcp::endpoint& endpoint) {
    m_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(m_ioService, endpoint);
    m_socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioService);
    m_work = std::make_shared<boost::asio::io_service::work>(m_ioService);
    m_acceptor->listen();
    m_acceptor->async_accept(*m_socket, boost::bind(&EchoAsyncServer::acceptHandler, this, boost::asio::placeholders::error));
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
        m_socket->async_read_some(boost::asio::buffer(ci->readBuffer), boost::bind(&EchoAsyncServer::readHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, ci->id));

        m_socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioService);
        m_acceptor->async_accept(*m_socket, boost::bind(&EchoAsyncServer::acceptHandler, this, boost::asio::placeholders::error));
    }
}

void EchoAsyncServer::readHandler(const boost::system::error_code& ec, std::size_t bytes_transferred, uint64_t socketId) {
    if (!ec) {
        if (!m_connectInfos.contains(socketId)) {
            return;
        }
        auto ci = m_connectInfos[socketId];
        std::string msg(ci->readBuffer.data(), bytes_transferred);
        std::cout << "receive id:" << socketId << " msg:" << msg << std::endl;
        ci->sendMsgs.push_back(std::make_shared<std::string>(msg));
        if (ci->sendMsgs.size() == 1) {
            ci->socket->async_send(boost::asio::buffer(*ci->sendMsgs.front()), boost::bind(&EchoAsyncServer::writeHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, ci->id));
        }
        ci->socket->async_read_some(boost::asio::buffer(ci->readBuffer), boost::bind(&EchoAsyncServer::readHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, ci->id));
    }
    else {
        // 断开连接
        std::cout << "disconnect1 id:" << socketId << std::endl;
        m_connectInfos.erase(socketId);
    }
}

void EchoAsyncServer::writeHandler(const boost::system::error_code &ec, std::size_t bytes_transferred, uint64_t socketId)
{
    if (!m_connectInfos.contains(socketId)) {
        return;
    }
    if (ec) {
        // 断开连接
        std::cout << "disconnect2 id:" << socketId << " " << ec.what() << std::endl;
        m_connectInfos.erase(socketId);
        return;
    }
    auto ci = m_connectInfos[socketId];
    assert(!ci->sendMsgs.empty());
    if (ci->sendMsgs.empty()) {
        return;
    }
    ci->sendMsgs.pop_front();
    if (!ci->sendMsgs.empty()) {
        ci->socket->async_send(boost::asio::buffer(*ci->sendMsgs.front())
                               ,boost::bind(&EchoAsyncServer::writeHandler
                                            , this
                                            , boost::asio::placeholders::error
                                            ,boost::asio::placeholders::bytes_transferred
                                            , ci->id));
    }
}