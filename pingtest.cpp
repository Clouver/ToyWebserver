//
// Created by Clouver on 2022/8/15.
//


#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <signal.h>
#include <functional>

using namespace std;

int main(void){
    char aa[6] = "12321";
    aa[2] = 0;
    string test(aa);
    cout<<test<<" "<<test.size()<<endl;

    return 0;

    string s;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in toAddr{};
    memset(&toAddr, 0, sizeof(toAddr));
    toAddr.sin_port = htons(8088);
    toAddr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &toAddr.sin_addr);
//    toAddr.sin_addr.s_addr = htonl(  )

    int ret = connect(fd, (sockaddr*)&toAddr, sizeof(toAddr));
    cout<<ret<<endl;
    char buf[513];
    while(true){
        cin>>s;
        if(s == "q")
            break;
        else{
            strcpy(buf, s.c_str());
            int x = write(fd, buf, strlen(buf));
            memset(buf, 0, sizeof(buf));

            read(fd, buf, 512);
            cout<<buf<<endl;
        }
    }
    close(fd);
}
