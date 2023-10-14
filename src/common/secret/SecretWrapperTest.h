//
// Created by Administrator on 2023/10/10.
//

#ifndef CPPSTUDY_SECRETWRAPPERTEST_H
#define CPPSTUDY_SECRETWRAPPERTEST_H
#include "SecretWrapper.hpp"

class SecretWrapperTest {
public:
    static void test() {
        std::srand(time(nullptr));
        char a[8] = "123123";
        SecretWrapper<char[8]> w = a;
        char b[8];
        w.get(b);
        printf("%s %s", a, b);
        SecretWrapper<char[8]> w2;
        w2 = w;
        SecretWrapper<char[8]> w3(w2);
        w2.get(b);
    }
};

#endif //CPPSTUDY_SECRETWRAPPERTEST_H
