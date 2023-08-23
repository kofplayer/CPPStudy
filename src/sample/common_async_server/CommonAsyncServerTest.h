//
// Created by Administrator on 2023/8/23.
//

#ifndef CPPSTUDY_COMMONASYNCSERVERTEST_H
#define CPPSTUDY_COMMONASYNCSERVERTEST_H

#include "CommonAsyncServer.h"
#include <boost/asio.hpp>
#include <iostream>

class CommonAsyncServerTest {
public:
    static void test() {

        boost::asio::io_service ioService;
        CommonAsyncServer svr;
        svr.onConnect([&svr, &ioService](CommonAsyncServer::connect_id_type connectId, const std::string & address){
            svr.sendMessage(connectId, address.data(), address.size());
            ioService.post([connectId, address]{
                std::cout << connectId << " connect, address: " << address << std::endl;
            });
        });
        svr.onMessage([&svr, &ioService](CommonAsyncServer::connect_id_type connectId, const char* data, size_t len){
            svr.sendMessage(connectId, data, len);
        });
        svr.onDisconnect([&ioService](CommonAsyncServer::connect_id_type connectId) {
            ioService.post([connectId]{
                std::cout << connectId << " disconnect" << std::endl;
            });
        });
        svr.start(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080), 4);
        boost::asio::deadline_timer timer1(ioService, boost::posix_time::seconds(50000000));
        timer1.async_wait([&svr](const boost::system::error_code &ec){
            svr.stop();
        });
        std::thread t([&ioService](){
            ioService.run();
        });
        t.join();
    }
};


#endif //CPPSTUDY_COMMONASYNCSERVERTEST_H
