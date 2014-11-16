#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "readfile.h"
#include "dvcalc.h"


using namespace std;


extern int *nbrlst;
extern int *nexthop;
extern int servers;
int hop;
char IPnew[INET_ADDRSTRLEN];
extern std::string *IPad;
int **matrix;
int change;
extern int *ncost;

int dvcal(int srvs,int nbors,short int myd,short int inid,server *mytbl,server *itbl)
{
  change=0;int *ctemp=new int[nbors];
  for (int i=0;i<srvs;i++)
  {
   matrix[inid][i]=itbl[i].cost;
  }

    for (int j=0; j<srvs; j++)
    {
        if (j==myd)
        {
            continue;
        }

       for (int i=0;i<nbors;i++)
       {
       if ( (nbrlst[i]-1)==j )
       {
       ctemp[i]=ncost[j];
       continue;
       }
       ctemp[i]=mytbl[nbrlst[i]-1].cost+matrix[nbrlst[i]-1][j];
       }
     // hop=*min_element(ctemp,ctemp+nbors);
   // cout<<*min_element(ctemp,ctemp+nbors)<<" ";
   
 
 if  (mytbl[j].cost!=*min_element(ctemp,ctemp+nbors))// &&((nexthop[j]-1)!=(distance(ctemp,min_element(ctemp,ctemp+nbors)))))
    {
    mytbl[j].cost=*min_element(ctemp,ctemp+nbors);
    nexthop[j]=nbrlst[distance(ctemp,min_element(ctemp,ctemp+nbors))];
    change=1;
    if (nexthop[j]==j+1)nexthop[j]=0;
    }


   if (mytbl[j].cost>999)
   mytbl[j].cost=999;
   
   }
   
    if (change==1)
    {
        print();
    }
    free(ctemp);
    return (change);
}

void setmatrix(int myd,server *mycost)
{
//Create matrix
 matrix=new int*[servers];
 for (int i=0;i<servers;i++)
 matrix[i]=new int[servers];


//Initialize to 999
    for (int i=0; i<servers; i++)
    {
    for (int j=0; j<servers; j++)
        {
        matrix[i][j]=999;

        if (i==j)
        matrix[i][j]=0;
        }
    }

    for (int i=0; i<servers; i++)
    {
    matrix[myd][i] =mycost[i].cost;
    }

}


void set_distance(int k)
{
for (int i=0;i<servers;i++)
{
matrix[k][i]=999;
}
}


/*void mprint()
{

 for (int i=0; i<servers; i++)
    {
    for (int j=0; j<servers; j++)
        {
        cout<<matrix[i][j]<<"\t";
        }
    cout<<endl;
    }

}
*/
