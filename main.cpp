//
// Created by 张锦 on 2023/2/22.
//

#include "sample/echo_async_server/EchoAsyncServer.h"

int main() {
    EchoAsyncServer svr;
    svr.start(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));
}