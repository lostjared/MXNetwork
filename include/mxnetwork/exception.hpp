#ifndef _MXNET_EXCEPT
#define _MXNET_EXCEPT

#include<string>
#include<iostream>

namespace mxnetwork {

    class Exception {
    public:
        Exception(const std::string &s) : txt{s} {}
        std::string text() const;
    protected:
        std::string txt;
    };
}

#endif
