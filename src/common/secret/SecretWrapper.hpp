//
// Created by Administrator on 2023/10/10.
//

#ifndef CPPSTUDY_SECRETWRAPEER_H
#define CPPSTUDY_SECRETWRAPEER_H
#include <random>

template<typename T>
class SecretWrapper {
public:
    SecretWrapper() : m_key(0) {
        memset(this, 0, sizeof (*this));
    }

    SecretWrapper(const SecretWrapper<T>& v) {
        T _v;
        v.get(_v);
        set(_v);
    }

    SecretWrapper(const T& v) {
        set(v);
    }

    SecretWrapper<T>& operator=(const T& v) {
        set(v);
        return *this;
    }

    SecretWrapper<T>& operator=(const SecretWrapper<T>& v) {
        T _v;
        v.get(_v);
        set(_v);
        return *this;
    }

    void set(const T& v) {
        m_key = (std::rand() % 0xFF);
        char * p = (char*)&v;
        for(int i=0; i < sizeof(T); ++i) {
            m_data[i] = (*p) ^ m_key;
            ++p;
        }
    }

    void get(T& v) const {
        char* p = (char*)&v;
        for(int i=0; i < sizeof(T); ++i) {
            *p = m_data[i] ^ m_key;
            ++p;
        }
    }
private:
    char m_data[sizeof(T)];
    char m_key;
};

#endif //CPPSTUDY_SECRETWRAPEER_H
