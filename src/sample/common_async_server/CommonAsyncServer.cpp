//
// Created by Administrator on 2023/8/22.
//

#include "CommonAsyncServer.h"

#include <memory>
#include <iostream>

CommonAsyncServer::CommonAsyncServer() {
    m_running = false;
    m_genConnectId = 1;
    m_ownIoService = false;
}

bool CommonAsyncServer::_start(const boost::asio::ip::tcp::endpoint &endpoint, int rwThreadCount,
                               const std::shared_ptr<boost::asio::io_service> &ioService) {
    if (m_running) {
        return false;
    }
    m_running = true;
    if (ioService) {
        m_ownIoService = false;
        m_ioService = ioService;
    } else {
        m_ownIoService = true;
        m_ioService = std::make_shared<boost::asio::io_service>();
        m_work = std::make_shared<boost::asio::io_service::work>(*m_ioService);
    }
    if (rwThreadCount > 0) {
        for (int i=0; i<rwThreadCount; ++i) {
            std::shared_ptr<Worker> worker(new Worker());
            worker->ownIoService = true;
            worker->ioService = std::make_shared<boost::asio::io_service>();
            m_workers.emplace_back(std::shared_ptr<Worker>(worker));
            worker->work = std::make_shared<boost::asio::io_service::work>(*worker->ioService);
            m_threadGroup.create_thread([worker](){
                worker->ioService->run();
            });
        }
    } else {
        std::shared_ptr<Worker> worker(new Worker());
        worker->ownIoService = false;
        worker->ioService = m_ioService;
        m_workers.emplace_back(std::shared_ptr<Worker>(worker));
    }
    m_genWorker = getWorkerByConnectId(m_genConnectId);
    m_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(*m_ioService, endpoint);
    m_socket = std::make_shared<boost::asio::ip::tcp::socket>(*m_genWorker->ioService);
    m_acceptor->listen();
    m_acceptor->async_accept(*m_socket, [this](const boost::system::error_code &ec) { acceptHandler(ec); });
    if (m_ownIoService) {
        m_threadGroup.create_thread([this](){
            m_ioService->run();
        });
    }
    return true;
}

bool CommonAsyncServer::start(const boost::asio::ip::tcp::endpoint &endpoint, int rwThreadCount) {
    return _start(endpoint, rwThreadCount, nullptr);
}

bool CommonAsyncServer::start(const boost::asio::ip::tcp::endpoint& endpoint, const std::shared_ptr<boost::asio::io_service> &ioService) {
    assert(ioService);
    return _start(endpoint, 0, ioService);
}

bool CommonAsyncServer::stop() {
    if (!m_running) {
        return false;
    }
    m_acceptor.reset();
    for (auto & worker : m_workers) {
        worker->work.reset();
        if (worker->ownIoService) {
            worker->ioService->stop();
        }
    }
    m_work.reset();
    if (m_ownIoService) {
        m_ioService->stop();
    }
    m_threadGroup.join();
    m_genWorker.reset();
    m_socket.reset();
    m_workers.clear();
    m_ioService->reset();
    m_running = false;
    return true;
}

void CommonAsyncServer::acceptHandler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        auto ci = std::make_shared<ConnectInfo>();
        connect_id_type connectId = m_genConnectId++;
        std::string address = m_socket->remote_endpoint().address().to_string();
        ci->socket = m_socket;
        ci->connectId = connectId;
        auto func = [this, worker = m_genWorker, ci](){
            worker->connectInfos.insert(std::make_pair(ci->connectId, ci));
            ci->socket->async_read_some(boost::asio::buffer(ci->readBuffer), [this, worker, ci] (const boost::system::error_code& ec, std::size_t bytes_transferred) { readHandler(ec, bytes_transferred, ci->connectId, worker); });
        };
        if (m_genWorker->ownIoService) {
            m_genWorker->ioService->post(func);
        } else {
            func();
        }
        m_genWorker = getWorkerByConnectId(m_genConnectId);
        m_socket = std::make_shared<boost::asio::ip::tcp::socket>(*m_genWorker->ioService);
        m_acceptor->async_accept(*m_socket, [this] (const boost::system::error_code &ec) { acceptHandler(ec); });
        if (m_onConnect) {
            m_onConnect(connectId, address);
        }
    }
}

std::shared_ptr<CommonAsyncServer::Worker> CommonAsyncServer::getWorkerByConnectId(connect_id_type connectId) {
    return m_workers[connectId % m_workers.size()];
}

void
CommonAsyncServer::readHandler(const boost::system::error_code &ec, std::size_t bytes_transferred, connect_id_type connectId,
                               const std::shared_ptr<Worker>& worker) {
    if (!ec) {
        if (!worker->connectInfos.contains(connectId)) {
            return;
        }
        std::shared_ptr<ConnectInfo> ci = worker->connectInfos[connectId];
        if (m_onMessage) {
            m_onMessage(connectId, ci->readBuffer.data(), bytes_transferred);
        }
        ci->socket->async_read_some(boost::asio::buffer(ci->readBuffer), [this, ci, worker] (const boost::system::error_code& ec, std::size_t bytes_transferred) { readHandler(ec, bytes_transferred, ci->connectId, worker); });
        
    }
    else {
        worker->connectInfos.erase(connectId);
        if (m_onDisconnect) {
            m_onDisconnect(connectId);
        }
    }
}

void
CommonAsyncServer::writeHandler(const boost::system::error_code &ec, std::size_t bytes_transferred, connect_id_type connectId,
                                const std::shared_ptr<Worker>& worker) {
    if (!worker->connectInfos.contains(connectId)) {
        return;
    }
    if (ec) {
        worker->connectInfos.erase(connectId);
        if (m_onDisconnect) {
            m_onDisconnect(connectId);
        }
        return;
    }
    std::shared_ptr<ConnectInfo> ci = worker->connectInfos[connectId];
    assert(!ci->sendMsgs.empty());
    if (ci->sendMsgs.empty()) {
        return;
    }
    assert(bytes_transferred == ci->sendMsgs.front()->size());
    ci->sendMsgs.pop_front();
    if (!ci->sendMsgs.empty()) {
        ci->socket->async_send(boost::asio::buffer(*ci->sendMsgs.front())
                ,[this, ci, worker](const boost::system::error_code &ec, std::size_t bytes_transferred) { writeHandler(ec, bytes_transferred, ci->connectId, worker); });
    }
}

void CommonAsyncServer::onConnect(const std::function<void(connect_id_type, const std::string &)>& callback) {
    m_onConnect = callback;
}

void CommonAsyncServer::onDisconnect(const std::function<void(connect_id_type)>& callback) {
    m_onDisconnect = callback;
}

void CommonAsyncServer::onMessage(const std::function<void(connect_id_type, const char*, size_t)>& callback) {
    m_onMessage = callback;
}

bool CommonAsyncServer::sendMessage(CommonAsyncServer::connect_id_type connectId, const char *data, size_t len) {
    auto worker = getWorkerByConnectId(connectId);
    auto func = [this, connectId, worker, msg = std::string(data, len)]() mutable {
        auto iter = worker->connectInfos.find(connectId);
        if (iter == worker->connectInfos.end()) {
            return;
        }
        auto & ci = iter->second;
        ci->sendMsgs.emplace_back(std::make_shared<std::string>(std::move(msg)));
        if (ci->sendMsgs.size() == 1) {
            ci->socket->async_send(boost::asio::buffer(*ci->sendMsgs.front())
                    , [this, ci, worker](const boost::system::error_code &ec, std::size_t bytes_transferred) { writeHandler(ec, bytes_transferred, ci->connectId, worker); });
        }
    };
    if (worker->ownIoService || m_ownIoService) {
        worker->ioService->post(func);
    } else {
        func();
    }
    return true;
}

bool CommonAsyncServer::close(CommonAsyncServer::connect_id_type connectId) {
    auto worker = getWorkerByConnectId(connectId);
    auto func = [connectId, worker]() mutable {
        auto iter = worker->connectInfos.find(connectId);
        if (iter == worker->connectInfos.end()) {
            return;
        }
        iter->second->socket->close();
    };
    if (worker->ownIoService || m_ownIoService) {
        worker->ioService->post(func);
    } else {
        func();
    }
    return true;
}
