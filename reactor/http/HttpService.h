//
// Created by Clouver on 2022/8/17.
//

#ifndef TOYWEBSERVER_HTTPSERVICE_H
#define TOYWEBSERVER_HTTPSERVICE_H

#include <string>
#include <map>
#include <functional>
#include <unordered_map>
#include <vector>
#include "../Service.h"

using namespace std;

// todo buffer!

// todo cleaner code!

class RequestHeaders{

public:
    map<string,string>headers;
    void clear();

    string method, url, param, version;

    unsigned long  parse(const Buffer& buf);
};

class ResponseHeaders{
    int sz;
    int bufsz;
    char* buf;
public:
    ResponseHeaders();
    ~ResponseHeaders();

    void addFirstLine(string num, string tips);

    void addHeader(string key, string value);

    unsigned long Write(int sk);
};

class HttpService: public Service {
public:
    bool multipart=false;
    Buffer multipartBuf;
    RequestHeaders h;
    string boundary;
    unsigned long contentLen;
    unsigned long targetLen;

    int SolveRequest(int sk, Buffer &buf) override;
    int picTransform(int sk);
    static int getStatic(int sk, string s);
    static int errorHandle(int sk);

    unsigned long readMultiPart(Buffer& buf, unsigned long shift);

    int getStaticBuffered(int sk, string s);

    int errorHandleBuffered(int sk);
};

class HttpServiceFactory : public ServiceFactory{
    std::shared_ptr<Service> create() override;
};

#endif //TOYWEBSERVER_HTTPSERVICE_H
