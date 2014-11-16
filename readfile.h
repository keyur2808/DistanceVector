#ifndef READFILE_H
#define READFILE_H

#include <string>
#include <vector>
#include<stdint.h>

class server
{
    public:
        uint32_t IP;
        short int port;
        short int id;//Server id
        short int cost;
        short int disabled;//Disabled status
};


server *readfile(char *);
void *get_in_addr(struct sockaddr *);
int myno();
void setip();

#endif // SERVER_H


/*
        server();
        virtual ~server();
        string GetIP() { return IP; }
        void SetIP(string val) { IP = val; }
        int Getport() { return port; }
        void Setport(int val) { port = val; }
        int Getid() { return id; }
        void Setid(int val) { id = val; }
    protected:
    private:
    */
