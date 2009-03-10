/**
 * @file: socket_trial_client.c
 * @project: lif12p2p
 * @author: Benjamin GUILLON, base sur le travail de Jim FROST (http://world.std.com/~jimf/papers/sockets/sockets.html).
 * @since: 9/03/2009
 * @version: 9/03/2009
 *
 * Module a but purement expérimental afin de bien comprendre comment marchent les sockets. Ne seras pas inclu dans la version finale
 * du projet de lif12p2p.
 */

/* Inclusion des Headers */

/* obligatory includes */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>


/* Definition des constantes */
#define PORTNUM 50000 /* random port number, we need something */
#define MAXHOSTNAME 25 /* max length for hostnames */

/* Declaration des fonctions */
void fireman(int);
void do_something(int);
void do_something_as_receiver(int);
void do_something_as_sender(int);
int establish(unsigned short portnum);
int get_connection(int s);
int call_socket(char *hostname, unsigned short portnum);
int read_data(int s, char *buf, int n);
int write_data(int s, char *buf, int n);


/* Programme principal */
int main()
{
    int s;

    if ((s= establish(PORTNUM)) < 0)
    {
        /* plug in the phone */
        perror("establish");
        exit(1);
    }

    signal(SIGCHLD, fireman); /* this eliminates zombies */

    do_something_as_sender(s);

    return 0;
}


/* as children die we should get catch their returns or else we get * zombies, A Bad Thing. fireman() catches falling children. */
void fireman(int i)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) ;

}


/* this is the function that plays with the socket. it will be called * after getting a connection. */
void do_something_as_receiver(int s)
{
    /* do your thing with the socket here : : */
    char * buf;
    int len=100;
    buf=malloc(100);

    for(;;)
    {
        read_data(s,buf,len);
        if(buf!="\0")
        {
            printf("%s\n",buf);
            buf="";
        }
    }

    free(buf);
}


void do_something_as_sender(int s)
{
    /* do your thing with the socket here : : */
    char * str=malloc(100);
    char * hostname=malloc(MAXHOSTNAME);


    printf("HOSTNAME: \n");
    scanf("%s",hostname);

    call_socket(hostname,PORTNUM);

    printf("Ecrivez quelque chose: \n");
    scanf("%s",str);
    write_data(s,str,strlen(str));

    free(str);
    free(hostname);
}


/* code to establish a socket; originally from bzs@bu-cs.bu.edu */
int establish(unsigned short portnum)
{
    char myname[MAXHOSTNAME+1];
    int s;
    struct sockaddr_in sa;
    struct hostent *hp;

    memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */
    gethostname(myname, MAXHOSTNAME); /* who are we? */
    hp= gethostbyname(myname); /* get our address info */

    if (hp == NULL) /* we don't exist !? */
        return(-1);

    sa.sin_family= hp->h_addrtype; /* this is our host address */
    sa.sin_port= htons(portnum); /* this is our port number */

    if ((s= socket(AF_INET, SOCK_STREAM, 0)) < 0) /* create socket */
        return(-1);

    if (bind(s,(struct sockaddr *)&sa,sizeof(struct sockaddr_in)) < 0)
    {
        close(s);
        return(-1); /* bind address to socket */
    }

    listen(s, 3); /* max # of queued connects */
    return(s);
}


/* wait for a connection to occur on a socket created with establish() */
int get_connection(int s)
{
    int t; /* socket of connection */

    if ((t = accept(s,NULL,NULL)) < 0) /* accept connection if there is one */
        return(-1);

    return(t);
}


int call_socket(char *hostname, unsigned short portnum)
{
    struct sockaddr_in sa;
    struct hostent *hp;
    int s;

    if ((hp= gethostbyname(hostname)) == NULL)
    {
        /* do we know the host's */
        errno= ECONNREFUSED; /* address? */
        return(-1); /* no */
    }

    memset(&sa,0,sizeof(sa));
    memcpy((char *)&sa.sin_addr,hp->h_addr,hp->h_length); /* set address */
    sa.sin_family= hp->h_addrtype;
    sa.sin_port= htons((unsigned short)portnum);

    if ((s= socket(hp->h_addrtype,SOCK_STREAM,0)) < 0) /* get socket */
        return(-1);

    if (connect(s,(struct sockaddr *)&sa,sizeof sa) < 0)
    {
        /* connect */
        close(s);
        return(-1);
    }

    return(s);
}


int read_data(int s, /* connected socket */ char *buf, /* pointer to the buffer */ int n /* number of characters (bytes) we want */ )
{
    int bcount; /* counts bytes read */
    int br; /* bytes read this pass */

    bcount= 0;
    br= 0;

    while (bcount < n)
    {
        /* loop until full buffer */
        if ((br= read(s,buf,n-bcount)) > 0)
        {
            bcount += br; /* increment byte counter */
            buf += br; /* move buffer ptr for next read */
        }
        else if (br < 0) /* signal an error to the caller */
            return(-1);
    }

    return(bcount);
}


int write_data(int s, /* connected socket */ char *buf, /* pointer to the buffer */ int n /* number of characters (bytes) we want */ )
{
    if ((write(s,buf,n)) < 0)
    {
        printf("erreur write\n");
        return -1;
    }

    return n;
}


/**
 * Fin du fichier
 */


