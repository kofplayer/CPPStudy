//
// Created by 张锦 on 2023/2/22.
//

#include <iostream>
#include "sample/common_async_server/CommonAsyncServer.h"
#include "common/secret/SecretWrapperTest.h"
#include "sample/echo_async_server/EchoAsyncServer.h"
#include "sample/common_async_client/CommonAsyncClient.h"

int main() {
    EchoAsyncServer svr;
    std::thread t([&svr]() {
        svr.start(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));
    });
    std::shared_ptr<CommonAsyncClient> client(new CommonAsyncClient());
    std::shared_ptr<boost::asio::io_service> ioService(new boost::asio::io_service());
    client->onConnect([client](CommonAsyncClient::ErrorCode ec) {
        if (ec != CommonAsyncClient::ErrorCode::Success) {
            return;
        }
        std::string s("hello world!");
        client->sendMessage(s.data(), s.length());
    });
    client->onMessage([client](const char* data, size_t len) {
        std::cout << std::string(data, len) << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string s("hello world!");
        client->sendMessage(s.data(), s.length());
    });

    client->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address_v4("127.0.0.1"), 8080), ioService);
    auto work = std::make_shared<boost::asio::io_service::work>(*ioService);
    ioService->run();
    return 0;
}