#ifndef TOOLS_CC
#define TOOLS_CC

#include "tools.h"

bool has_str(const u_char *p, const char *str) {
    if(!p || !str)
        return false;

    for(int i = 0; i < strlen(str); i++)
        if(*(p + i) != *(str + i))
            return false;
    return true;
}

u_char *get_loc_ip() {
    ifaddrs * ifAddrStruct = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct != NULL) {
        if (ifAddrStruct->ifa_addr->sa_family == AF_INET) {
            tmpAddrPtr = &((sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if(ifAddrStruct->ifa_name[0] == 'e'
                && !has_str((const u_char *)"127.0.0.1", addressBuffer)) {
                    u_char *tmp = new u_char[strlen(addressBuffer)];
                    strcpy((char *)tmp, addressBuffer);
                    return tmp;
                }
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }
    return NULL;
}

int get_num(const u_char* p, const char terminator) {
    int num = 0;
    const u_char* c = p;
    while(*c != terminator) {
        if(*c >= '0' && *c <= '9')
            num = num * 10 + (int)(*c) - 48;
        c++;
    }
    return num;
}


#endif
