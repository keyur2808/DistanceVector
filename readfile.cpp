#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include "readfile.h"
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

extern int servers;
extern int nbrs;

std::string *IPad;

struct sockaddr_in ip;

using namespace std;

int x,y;
int a=0;
server *srv;
int *nbrlst;
int self;
int id;
int *ncost;

server *readfile(char *fl)
{
    ifstream fin;
    fin.open(fl,ios::in);
    int j=0;
    while(!fin.eof())
    {
        if (j==0)
        {
            fin>>servers;
            srv=new server[servers];
            memset(srv,0,servers*sizeof (server));
            ncost=new int[servers];
            memset(ncost,999,servers*sizeof (server));
            IPad=new std::string[servers];

        }
        if (j==1)
        {
            fin>>nbrs;
            nbrlst=new int[nbrs];
            cout<<servers<<endl;
        }
        if ( (j>1) && (j<servers+2) )
        {
            fin>>id;
            srv[id-1].id=id;
            fin>>IPad[id-1]>>srv[id-1].port;
        }
        if (j==(servers+nbrs+2)) break;
        if (j>servers+1)
        {
            fin>>y>>x;
            fin>>srv[x-1].cost;
            nbrlst[a]=x;
            ncost[x-1]=srv[x-1].cost;
            a++;
            //cout<<x<<y<<endl;
        }


        j++;
    }
    fin.close();
    return srv;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int myno()
{
    //Set non-neighbor cost to infinity
    for (int m=0; m<servers; m++)
    {
        if (srv[m].cost==0) srv[m].cost=999;

    }



    char sl[INET6_ADDRSTRLEN];//Get my ip address here
    int sockfdip;
    char GoogleDns[]="8.8.8.8";
    socklen_t len=sizeof ip;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    inet_pton(AF_INET,GoogleDns,&serv.sin_addr.s_addr);
    serv.sin_port = htons(53);
    if((sockfdip=socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        perror("socket:");
    }
    if (connect(sockfdip,(struct sockaddr*)&serv,sizeof serv) == -1)
    {
        close(sockfdip);
        perror("client: connect");
    }
    getsockname(sockfdip,(struct sockaddr *)&ip , &len);
    inet_ntop(AF_INET, get_in_addr((struct sockaddr *)&ip),sl, sizeof sl);
    close(sockfdip);
    cout<<"My IP is " <<sl<<endl;


    //Find my id
    for (int m=0; m<servers; m++)
    {
        if ( (strcmp(sl,IPad[m].c_str())==0) || (strcmp("127.0.0.1",IPad[m].c_str())==0) )
        {
            self=m;
            srv[m].cost=0;

        }
    }
    return self;
}

void setip()
{
for (int m=0;m<servers;m++)
{
inet_pton(AF_INET,IPad[m].c_str(),&srv[m].IP);
}
}
