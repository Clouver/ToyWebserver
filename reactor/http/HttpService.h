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

using namespace std;

class RequestHeaders{

public:
    map<string,string>headers;
    void clear(){
        headers.clear();
        method=url=param=version = "";
    }

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

    int Write(int sk);
};

class HttpService {
public:
    bool multipart=false;
    vector<char> multipartBuf;
    RequestHeaders h;
    string boundary;
    unsigned long contentLen;
    unsigned long targetLen;
    void SolveRequest(int sk, const vector<char>& buf);
    void picTransform(int sk);
    void getStatic(int sk, string s);

    int readMultiPart(int sk, const vector<char>& buf, int shift){
        for(unsigned long i=shift; i<buf.size(); i++) // 第一个请求，除了header的剩余
            multipartBuf.push_back(buf[i]);
        contentLen += buf.size()-shift;
        return buf.size();
    }
};

#endif //TOYWEBSERVER_HTTPSERVICE_H
