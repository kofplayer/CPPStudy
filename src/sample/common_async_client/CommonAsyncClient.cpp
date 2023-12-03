//
// Created by Administrator on 2023/9/16.
//

#include "CommonAsyncClient.h"
#include <boost/bind/bind.hpp>

bool CommonAsyncClient::connect(const boost::asio::ip::tcp::endpoint &endpoint,
                                const std::shared_ptr<boost::asio::io_service> &ioService) {
    if (m_status != Status::None) {
        return false;
    }
    m_status = Status::Connecting;
    m_socket = std::make_shared<boost::asio::ip::tcp::socket>(*ioService);
    m_socket->async_connect(endpoint, [this](const boost::system::error_code& ec) {
       if(!ec) {
           m_status = Status::Connected;
           if (m_onConnect) {
               m_onConnect(ErrorCode::Success);
           }
           m_socket->async_read_some(boost::asio::buffer(m_receiveBuffer),
                                     [this](const boost::system::error_code& ec, std::size_t bytes_transferred) { readHandler(ec, bytes_transferred); });
       } else {
           onError(ErrorCode::ConnectError);
       }
    });
    return true;
}

void CommonAsyncClient::onError(CommonAsyncClient::ErrorCode ec) {
    if (m_status == Status::None) {
        assert(false);
        return;
    }
    m_socket->close();
    m_socket.reset();
    m_sendMessages.clear();
    Status status = m_status;
    m_status = Status::None;
    switch (status) {
        case Status::None:
            break;
        case Status::Connecting:
            if (m_onConnect) {
                m_onConnect(ec);
            }
            break;
        case Status::Connected:
            if (m_onDisconnect) {
                m_onDisconnect();
            }
            break;
    }
}

void CommonAsyncClient::onConnect(const CommonAsyncClient::OnConnectedCallBack &callback) {
    m_onConnect = callback;
}

bool CommonAsyncClient::close() {
    if (m_status == Status::None) {
        return false;
    }
    if (m_status == Status::Connecting) {
        m_socket->close();
        m_socket.reset();
        return true;
    }
    if (m_status == Status::Connected) {
        m_socket->close();
        m_socket.reset();
        return true;
    }
    return false;
}

void CommonAsyncClient::onDisconnect(const CommonAsyncClient::OnDisconnectedCallBack &callback) {
    m_onDisconnect = callback;
}

void CommonAsyncClient::onMessage(const CommonAsyncClient::OnMessageCallBack &callback) {
    m_onMessage = callback;
}

bool CommonAsyncClient::sendMessage(const char *data, size_t len) {
    if (m_status != Status::Connected) {
        assert(false);
        return false;
    }
    auto sendMsg = std::make_shared<std::string>(std::string(data, len));
    m_sendMessages.emplace_back(sendMsg);
    if (m_sendMessages.size() == 1) {
        m_socket->async_send(boost::asio::buffer(*m_sendMessages.front())
                , [this, sendMsg](const boost::system::error_code &ec, std::size_t bytes_transferred) { writeHandler(ec, bytes_transferred, sendMsg); });
    }
    return true;
}

CommonAsyncClient::Status CommonAsyncClient::getStatus() {
    return m_status;
}

void CommonAsyncClient::readHandler(const boost::system::error_code &ec, std::size_t bytes_transferred) {
    if (m_status != Status::Connected) {
        assert(false);
        return;
    }
    if (ec) {
        onError(ErrorCode::ReadError);
        return;
    }
    if (m_onMessage) {
        m_onMessage(m_receiveBuffer.data(), bytes_transferred);
    }
    m_socket->async_read_some(boost::asio::buffer(m_receiveBuffer), [this](const boost::system::error_code& ec, std::size_t bytes_transferred) { readHandler(ec, bytes_transferred); });
}

void CommonAsyncClient::writeHandler(const boost::system::error_code &ec, std::size_t bytes_transferred,
                                     std::shared_ptr<std::string> data) {
    if (m_sendMessages.empty() || m_sendMessages.front().get() != data.get()) {
        assert(false);
        return;
    }
    if (ec) {
        onError(ErrorCode::WriteError);
        return;
    }
    assert(m_sendMessages.front()->size() == bytes_transferred);
    m_sendMessages.pop_front();
    if (!m_sendMessages.empty()) {
        std::shared_ptr<std::string> sendMsg = m_sendMessages.front();
        m_socket->async_send(boost::asio::buffer(*m_sendMessages.front())
                , [this, sendMsg](const boost::system::error_code &ec, std::size_t bytes_transferred) { writeHandler(ec, bytes_transferred, sendMsg); });
    }
}
