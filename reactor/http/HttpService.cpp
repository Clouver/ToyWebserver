//
// Created by Clouver on 2022/8/17.
//

#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include "HttpService.h"
#include "tools.h"
#include "../img/ImageTrans.h"

bool ECHO_REQUEST_INFO = false;
static bool ECHO_HEADERS = false;

static string RES_DIR = "./res";
static string default_file = "/404.html";

void RequestHeaders::clear(){
    headers.clear();
    method=url=param=version = "";
}

unsigned long RequestHeaders::parse(const vector<char>& buf){
    int i=0, d=0;

    // method 空格结尾
    while(buf[i+d]!=' ')
        d++;
    method = string(buf.begin()+i, buf.begin()+i+d );
    i+=d+1;

    d=0;
    while(buf[i+d]!=' ')
        d++;
    string fullUrl = string(buf.begin()+i, buf.begin()+i+d );
    string::size_type pi = fullUrl.find('?');
    url = fullUrl.substr(0, pi);
    if(pi < fullUrl.size())
        param = fullUrl.substr( pi+1, fullUrl.size()-pi-1);
    i+=d+1;

    d=0;
    while(buf[i+d]!='\n')
        d++;
    version = string(buf.begin()+i, buf.begin()+i+d );
    i+=d+1;
    // headers
    while(true){
        string line;
        d=0;
        while(buf[i+d]!='\r')
            d++;
        line = d==0?"":string(buf.begin()+i, buf.begin()+i+d );
        i+=d+2;
        if(line.empty())
            break;
        else{
            string::size_type x = line.find(':');
            string key = line.substr(0, x);
            string val = line.substr(x+2, line.size()-x-2);
            headers[key] = val;
        }
    }

//    if ( ECHO_HEADERS ){
//        // ECHO
//        cout<<"HTTP version:    "<<version<<endl;
//        cout<<"Request Method:  "<<method<<endl;
//        cout<<"Target Resource: "<<url<<endl;
//        cout<<"Params:          "<<param<<endl;
//        cout<<"Headers:         "<<endl;
//
//        for(auto & header : headers){
//            cout<<"                 "<<header.first.substr(0,15);
//            if(header.first.size() > 15 )
//                cout<<"...";
//            else
//                for(int i=0; i<15-(header.first.size())+3; i++)
//                    cout<<' ';
//            cout<<"\t:\t"<<header.second.substr(0, 20);
//
//            if(header.second.size() > 20)
//                cout<<"...";
//
//            cout<<endl;
//
//            if(header.first == "Accept")
//                cout<<"                 "<<"客户端接收文件类型"<<endl;
//            else if( header.first == "Accept-Encoding")
//                cout<<"                 "<<"编码类型"<<endl;
//            else if( header.first == "Accept-Language")
//                cout<<"                 "<<"语言"<<endl;
//            else if( header.first == "Cache-Control")
//                cout<<"                 "<<"缓存控制"<<endl;
//            else if( header.first == "Connection")
//                cout<<"                 "<<"是否保持连接"<<endl;
//            else if( header.first == "Host")
//                cout<<"                 "<<"主机"<<endl;
//            else if( header.first == "User-Agent")
//                cout<<"                 "<<"用户代理(浏览器)"<<endl;
//            else if( header.first == "Referer")
//                cout<<"                 "<<"提供者"<<endl;
//            else
//                cout<<"                 "<<"..."<<endl;
//        }
//    }
    return i;
}

ResponseHeaders::ResponseHeaders(){
    sz=0;
    bufsz = 4096;
    buf = new char[bufsz];
}

ResponseHeaders::~ResponseHeaders(){
    delete[] buf;
}

void ResponseHeaders::addFirstLine(string num, string tips){
    string ver = "HTTP/1.0";
    for(int i=0; i<ver.size(); i++,sz++)
        buf[sz]=ver[i];
    buf[sz++] = ' ';
    for(int i=0; i<num.size()&&sz<bufsz-8; i++,sz++)
        buf[sz] = num[i];
    buf[sz++] = ' ';
    for(int i=0; i<tips.size()&&sz<bufsz-8; i++,sz++)
        buf[sz] = tips[i];
    buf[sz++] = '\n';
}

void ResponseHeaders::addHeader(string key, string value){
    for(int i=0; i<key.size()&&sz<bufsz-8; i++,sz++)
        buf[sz] = key[i];
    buf[sz++] = ':';
    buf[sz++] = ' ';

    for(int i=0; i<value.size()&&sz<bufsz-8; i++,sz++)
        buf[sz] = value[i];
    buf[sz++] = '\n';
}

unsigned long ResponseHeaders::Write(int sk){
    buf[sz++] = '\n';
    buf[sz] = 0;
    return send(sk, buf, sz, MSG_NOSIGNAL);
}

// Send static file "s" in "RES_DIR" to "sk". The file must be in "files" or default_file.
void HttpService::getStatic(int sk, string s){

    if(s=="/")
        s+="index.html";

    // File name
    s = RES_DIR+s;

    // Open file & Get size
    FILE *fp = fopen(s.c_str(),"r");
    if(fp == nullptr ){
        s= RES_DIR+default_file;
        fp = fopen(s.c_str(), "r");
        if(fp == nullptr ){
            std::cout<<"not found "<<s<<endl;
            return;
        }
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Make response headers & Write headers
    ResponseHeaders rh;
    if(s == RES_DIR+default_file)
        rh.addFirstLine("404", "Not Found");
    else
        rh.addFirstLine("200", "OK");

    rh.addHeader("Server", "ToyWebserver");
    rh.addHeader("Connection", "keep-alive");
    rh.addHeader("Content-Type", getType(s));

    char sz_s[1024];
    sprintf(sz_s, "%ld", fileSize);
    rh.addHeader("Content-Length", sz_s);

    rh.Write(sk);

    // Write file
    char buffer[256];
    while(!feof(fp)){
        unsigned long readed = fread(buffer, sizeof(char), 256, fp);
        send(sk, buffer, readed, MSG_NOSIGNAL);
    }
    fclose(fp);
}

// preRead length max == READ_BUFFER_SIZE
void HttpService::SolveRequest(int sk, const vector<char>& buf){
    // set SO_LINGER for close socket

//    struct linger so_linger;
//    so_linger.l_onoff = true;
//    so_linger.l_linger = 10;
//    setsockopt(sk,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));

    // Read request

    if(multipart){
        unsigned long ret = readMultiPart(buf, 0);
        if(contentLen == targetLen || ret == 0){
            if(h.method=="POST" && h.url=="/picTrans"){
                picTransform(sk); //todo
            }
            multipart = false;
        }
        return;
    }

    // 不是multipart，则是新请求
    // 新请求先清空之前东西
    // todo 这是给keep alive 准备的
    multipartBuf.clear();
    contentLen = 0;
    boundary.clear();
    h.clear();
    unsigned long end = h.parse(buf);

    if(h.headers.count("Content-Type")!=0&& h.headers["Content-Type"].find("multipart") != h.headers["Content-Type"].size()){
        multipart = true;
        string c = h.headers["Content-Type"];
        unsigned long bi = c.find("boundary=");
        boundary = c.substr(bi+9, c.size()-bi-9);

        readMultiPart(buf, end);
        char* tmp;
        targetLen = strtol(h.headers["Content-Length"].c_str(), &tmp, 0);
    }


    // 当前就两个功能，静态文件的获取 和 PicToChar
    if(h.method=="GET"){
        getStatic(sk, h.url);
    }
    else if(h.method=="POST" && h.url=="/picTrans"){
        if(multipartBuf.size() == targetLen)
            picTransform(sk);
        else
            multipart = true;
    }

}

void HttpService::picTransform(int sk){
    // 找开头 boundary
    // content body 有额外文件信息 用 回车符（+换行）作为标识，直接跳过。
    int st=0, rcnt=4;
    while(rcnt>0){
        if(multipartBuf[st] == '\r'){
            st++;
            rcnt--;
        }
        st++;
    }

    // 找结尾 boundary
    unsigned long ed=multipartBuf.size()-boundary.size();
    while(ed>0){
        bool ok=true;
        for(int j=0; j<boundary.size(); j++){
            if(multipartBuf[ed+j] != boundary[j]){
                ok = false;
                break;
            }
        }
        if(ok)
            break;
        ed--;
    }

    // 文件本体
    vector<char>img;
    for(int i=st; i<ed; i++)
        img.push_back(multipartBuf[i]);

    // 调用图片处理函数
    ImageTrans trans;
    vector<vector<char>> charImg= trans.loadpng(img);
    vector<char>toWrite;
    for(int i=0; i<charImg.size(); i++){
        for(int j=0; j<charImg[i].size(); j++){
            toWrite.push_back(charImg[i][j]);
        }
        toWrite.push_back('\n');
    }

    // response 的 header
    ResponseHeaders rh;
    rh.addFirstLine("200", "OK");

    rh.addHeader("Server", "ToyWebserver");
    rh.addHeader("Connection", "keep-alive");
    rh.addHeader("Content-Type", "text/plain;charset=UTF-8");

    rh.addHeader("Content-Length", to_string(toWrite.size()));
    rh.Write(sk);

    // 写图片字符
    char buffer[257];
    int n=0;
    while(n<toWrite.size()){
        int i=0;
        for(; n+i<toWrite.size() && i<256; i++)
            buffer[i] = toWrite[n+i];
        buffer[i]=0;
        n+=i;
        send(sk, buffer, i, MSG_NOSIGNAL);
    }

}

unsigned long HttpService::readMultiPart(const vector<char>& buf, unsigned long shift){
    for(unsigned long i=shift; i<buf.size(); i++) // 第一个请求，除了header的剩余
        multipartBuf.push_back(buf[i]);
    contentLen += buf.size()-shift;
    return buf.size();
}