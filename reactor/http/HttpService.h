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

    unsigned long  parse(const vector<char>& buf);
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
    vector<char> multipartBuf;
    RequestHeaders h;
    string boundary;
    unsigned long contentLen;
    unsigned long targetLen;
    void SolveRequest(int sk, const vector<char>& buf) ;
    void SolveRequest(int sk, Buffer &buf) override;
    void picTransform(int sk);
    static void getStatic(int sk, string s);
    static void errorHandle(int sk);

    unsigned long readMultiPart(const vector<char>& buf, unsigned long shift);
};

class HttpServiceFactory : public ServiceFactory{
    void create(std::shared_ptr<Service>& spService) override;
};

#endif //TOYWEBSERVER_HTTPSERVICE_H
