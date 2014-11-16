#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ctime>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <arpa/inet.h>
#include <list>
#include "readfile.h"
#include "dvcalc.h"

#define inf 999

extern int *nbrlst;
extern std::string *IPad;

using namespace std;

int t=0;
char *buffer;
int bflag = 0;
char *cvalue = NULL;
int *rcvup;
int *nexthop;
extern int *ncost;

int sockfd;
struct sockaddr_in nbr;
struct sockaddr_in me;
struct timeval timeout;
socklen_t len;
int c;
int yes=1;
char BUF[100];
int desc_ready;
int info=0;
fd_set master_set,working_set;

server *hst;
server *itable;
int myid;
int servers=0;
int nbrs=0;
int packets=0;
char remoteIP[INET_ADDRSTRLEN];

void qwait()
{
cout<<"Crashed"<<endl;
while(1)
{
scanf("%s",BUF);
if (strcasecmp(BUF,"exit")==0)
exit(EXIT_SUCCESS);
}
}

void process_cmd(char cmd[])
{

    if (strcasecmp(cmd,"exit")==0)
    {
        close(sockfd);
        exit(EXIT_SUCCESS);
        return;
    }

   if (strcasecmp(cmd,"crash")==0)
    {
        FD_CLR(0,&master_set);
        FD_CLR(sockfd,&master_set);
        close(sockfd);
        qwait();
        return;
    }


    if (strcasecmp(cmd,"display")==0)
    {
        print();
        return;
    }

    if (strcasecmp(cmd,"step")==0)
    {
        update();
        timeout.tv_sec=bflag;
        timeout.tv_usec=0;
        return;
    }

    if (strcasecmp(cmd,"packets")==0)
    {
        cout<<packets<<endl;
        packets=0;
        return;
    }


    if(strstr(cmd,"disable")!=NULL)
    {
        scanf("%s",BUF);
        int a=atoi(BUF);
        int i;
        for (i=0; i<nbrs; i++)
        {
            if(a==nbrlst[i])
            {
                cout<<"disable "<<a<<" -Success"<<endl;
                hst[a-1].disabled=1;
                break;
            }
        }
        if (i==nbrs)
        {
            cout<<"disable "<<a<<" -Error"<<endl;
            cout<<"No such neighbor"<<endl;
        }

        return;
    }

    if(strstr(cmd,"update")!=NULL)
    {
        int i=0;
        scanf("%s",BUF);
        int a=atoi(BUF);
        if (a!=(myid+1))
        {
            cout<<"update-Error"<<endl;
            cout<<"Cannot change links not directly connected"<<endl;
            return;
        }
        scanf("%s",BUF);
        int b=atoi(BUF);
        for (i=0; i<nbrs; i++)
        {
            if(b==nbrlst[i])
            {
                break;
            }
        }
        int cost;
        if (i<nbrs)
        {
            scanf("%s",BUF);
            if(strcasecmp(BUF,"inf")==0)
            {
                cost=999;
            }
            else
                cost=atoi(BUF);
        }
        else
        {
            cout<<"Update-Error"<<endl;
            cout<<b<<" No such neighbor"<<endl;
            return;
        }

        hst[b-1].cost=cost;
        ncost[b-1]=cost;
        nexthop[b-1]=0;
        dvcal(servers,nbrs,myid,myid,hst,hst);        
        update();
        print();
    }

}

void listen()
{
    int down=0;
    timeout.tv_sec=bflag;
    timeout.tv_usec=0;
    len=sizeof nbr;
    short int incid;
    do
    {
        memcpy(&working_set, &master_set, sizeof(master_set));
        printf("\nWaiting for updates...\n");
        cout<<"server>";
        cerr<<"";
        info = select(4, &working_set, NULL, NULL,&timeout);

        if (info < 0 && errno!=EINTR)
        {
            perror("\nselect() failed");
            break;
        }
        if (info == 0)
        {   timeout.tv_sec=bflag;
            timeout.tv_usec=0;
            for (int i=0; i<nbrs; i++)
            {
                if  (rcvup[i]<3)
                {
                    rcvup[i]++;
                }
                if (rcvup[i]==3)
                {
                    hst[nbrlst[i]-1].cost=999;
                    down=1;
                    rcvup[i]=rcvup[i]+1;
                    ncost[nbrlst[i]-1]=999;
                    set_distance(nbrlst[i]-1);
               }

            }
            if (down==1)
            {
            down=0;
            print();

            }
             update();
        }

        desc_ready = info;
        for (int i=0; i <= 3 && desc_ready > 0; ++i)
        {

            if (FD_ISSET(i, &working_set))
            {
                desc_ready -= 1;

                if(i==0)
                {


                    memset(BUF,0,sizeof BUF);
                    scanf("%s",BUF);
                    process_cmd(BUF);



                }

                if (i == sockfd)
                {
                    buffer=(char *)malloc(1*t);
                    memset(buffer,0,t);
                    c=recvfrom(sockfd,buffer,t,0,(struct sockaddr*)&nbr,&len);
                    if (c<0)
                    {
                        perror("rcv");
                        break;
                    }
                    inet_ntop(nbr.sin_family,(struct sockaddr_in*)&nbr.sin_addr,remoteIP,sizeof remoteIP);

                    buffer=buffer+sizeof(short);
                    memcpy(&incid,buffer,sizeof(short));
                    buffer=buffer+sizeof(short);

                    if (incid>servers)
                    {
                    continue;         
                    }                    
 
                    if (hst[incid-1].disabled==1)
                    {
                    buffer=buffer-(2*sizeof(short));
                    free(buffer);
                    continue;
                    }
                    printf("\n Listening socket has recvd update from ");
                    cout<<remoteIP<<" via port "<<htons(nbr.sin_port)<<" with Id "<<incid<<endl;
                    packets++;

                    for (int i=0; i<nbrs; i++)
                    {
                        if (nbrlst[i]==incid)
                        {
                            rcvup[i]=0;
                        }
                    }

                    itable=new server[servers];
                    memcpy(itable,buffer,servers*sizeof (server));
                    int s=dvcal(servers,nbrs,myid,incid-1,hst,itable);
                    free(itable);
                    buffer=buffer-(2*sizeof(short));
                    free(buffer);
                   /* if (s==1)
                    {
                        s==0;
                        cout<<"Sending Updates"<<endl;
                        update();
                    }
                  */
                }

            }

        }

    }
    while(1);
}

int main(int argc,char *argv[])
{
  if (argc!=5)
  {
  cout<<"Usage:./server -t topologyfile -i timeoutinterval"<<endl;
  exit(1);
  }

    int index;
    int d;

    opterr = 0;

    while ((d = getopt (argc, argv, "t:i:")) != -1)
        switch (d)
        {
        case 'i':
            bflag = atoi(optarg);
            break;
        case 't':
            cvalue = optarg;
            break;
        case '?':
            if ( (optopt == 't') || (optopt=='i') )
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            return 1;
        default:
            abort ();
        }

    printf ("Timeout = %d, File = %s\n",
            bflag, cvalue);

    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);

//Process file & Setup socket
    hst=readfile(cvalue);
    myid=myno();
    setip();
    rcvup=new int[nbrs];
    setmatrix(myid,hst);
    nexthop=new int[servers];
    memset(nexthop,0,servers*sizeof(int));
    memset(rcvup,0,nbrs*sizeof(int));
    t=(2*sizeof(short)+servers*sizeof(server));
    cout<<"My Id is "<<myid+1<<endl;
    cout<<"Listening Port-"<<hst[myid].port<<endl;
    cout<<"\t\t\tInitial Topology"<<endl;
    print();
    FD_SET(0,&master_set);
    info= fcntl(0, F_SETFL,O_NONBLOCK);
    if (info < 0)
    {
        perror("fl failed");
        close(0);
        exit(-1);
    }
    memset(&me,0,sizeof me);
    me.sin_family=AF_INET;
    me.sin_addr.s_addr=htonl(INADDR_ANY);
    me.sin_port=htons(hst[myid].port);
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    c = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
    c = bind(sockfd,(struct sockaddr *)&me,sizeof me);
    if (c<0)perror("er:");
    FD_SET(sockfd, &master_set);
    cout<<"Sending Inital Config"<<endl;
    //cout<<t<<endl;
    update();
//    cout<<"Matrix"<<endl;
//    mprint();
    listen();
    return 0;
}


void update()
{   cout<<endl;
    short int fields=5;
    buffer=(char *)malloc(1*t);
    memset(buffer,0,t);
    memcpy(buffer,&fields,sizeof (short));
    buffer=buffer+sizeof(short);
    memcpy(buffer,&hst[myid].id,sizeof(short));
    buffer=buffer+sizeof(short);
    memcpy(buffer,hst,servers*sizeof(server));
    buffer=buffer-(2*sizeof(short));
    for (int i=0; i<nbrs; i++)
    {
        if (hst[nbrlst[i]-1].disabled==0)
       {
        memset(&nbr,0,sizeof nbr);
        nbr.sin_family=AF_INET;
        nbr.sin_addr.s_addr=(hst[nbrlst[i]-1].IP);
        nbr.sin_port=htons(hst[nbrlst[i]-1].port);
        cout<<"Sending Update to-"<<nbrlst[i]<<" "<<inet_ntop(AF_INET,&hst[nbrlst[i]-1].IP,remoteIP,sizeof remoteIP)<<endl;
        sendto(sockfd,buffer,t,0,(struct sockaddr*)&nbr,sizeof nbr);
       }

    }
    free(buffer);
}


void print()
{
    cout<<"Servers-"<<servers<<endl;
    cout<<"No of neighbors-"<<nbrs<<endl;
    cout<<"Id\t"<<"IP Address\t"<<"Port\t"<<"Cost\t"<<"Next Hop "<<endl;
    for (int k=0; k<servers; k++)
    {
        if (hst[k].id==0)continue;
        if (hst[k].cost<999)
        cout<<hst[k].id<<"\t"<<IPad[k]<<"\t"<<hst[k].port<<"\t"<<hst[k].cost<<"\t"<<nexthop[k]<<endl;
        else
        cout<<hst[k].id<<"\t"<<IPad[k]<<"\t"<<hst[k].port<<"\t"<<"inf"<<"\t"<<nexthop[k]<<endl;
    }


}
