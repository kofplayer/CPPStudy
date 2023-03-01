//
// Created by 张锦 on 2023/2/22.
//

#include <functional>
#include <iostream>
#include "./common/thread/ThreadPool.h"

int foo(int para) {
    return para;
}

void fff() {
    for (int i = 30; i < 40; ++i) {
        printf("%d\n", i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    // std::function 包装了一个返回值为 int, 参数为 int 的函数
    std::function<int(int)> func = foo;

    int important = 10;
    std::function<int(int)> func2 = [&](int value) -> int {
        return 1+value+important;
    };
    std::cout << func(10) << std::endl;
    std::cout << func2(10) << std::endl;

    ThreadPool tp;
    tp.start(2);
    tp.exec([](){
        for (int i = 0; i < 10; ++i) {
            printf("%d\n", i);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    tp.exec([](){
        for (int i = 10; i < 20; ++i) {
            printf("%d\n", i);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    tp.exec([](){
        for (int i = 20; i < 30; ++i) {
            printf("%d\n", i);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    tp.exec(fff);

    tp.stop();
}