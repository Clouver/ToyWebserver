
#include "tools.h"

// Tool functions
void error(const char *msg){
    perror(msg);
    exit(0);
}

void readString(char* to, char endFlag, int limit, char* buf, int& idx, int bufEnd){
    to[0]='\0';
    for(int i=0; i<limit && idx<bufEnd && buf[idx]!=endFlag; i++, idx++)
        to[i] = buf[idx], to[i+1] = '\0';
}

void skipSpace(char* buf, int& idx, int bufEnd){
    // 32 is ' ', all lower ascii letters are invisible. Such as 13, \t and \n.
    while(idx<bufEnd &&  buf[idx]<=32 )
        idx++;
}

std::string getType(std::string s){
    int t=-1;
    for(int i=s.size()-1; i>=0; i--)
        if(s[i] == '.'){
            t = i;
            break;
        }
    if( t < 0)
        return "\0";


    /**************************************************************
     *                                                            *
     *       More file type to be extended.                       *
     *                                                            *
     **************************************************************/
    std::string tail = s.substr(t+1);
    //cout<<"tail: "<<tail<<endl;
    if(tail == "html")
        return "text/"+tail;
    else if( tail == "ico" || tail == "png" || tail == "jpg" || tail == "jpeg" || tail == "gif") // more
        return "image/"+tail;
    else
        return "text/plain";
}
