//
// Created by Administrator on 2023/8/23.
//

#ifndef CPPSTUDY_ECHOASYNCSERVERTEST_H
#define CPPSTUDY_ECHOASYNCSERVERTEST_H

#include "EchoAsyncServer.h"

class EchoAsyncServerTest {
public:
    static void test() {
        EchoAsyncServer svr;
        std::thread t([&svr](){
            svr.start(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));
        });
        std::this_thread::sleep_for(std::chrono::seconds(5));
        svr.stop();
        t.join();
    }
};


#endif //CPPSTUDY_ECHOASYNCSERVERTEST_H
