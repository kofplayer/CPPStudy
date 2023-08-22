//
// Created by 张锦 on 2023/2/22.
//
#include "sample/echo_async_server/EchoAsyncServer.h"

int main() {
    EchoAsyncServer svr;
    std::thread t([&svr](){
        svr.start(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));
    });
    std::this_thread::sleep_for(std::chrono::seconds(5));
    svr.stop();
    t.join();
    return 0;
}