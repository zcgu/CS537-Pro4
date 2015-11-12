#include "cs537.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too

pthread_mutex_t mutexT;
pthread_cond_t empty, full;
int head =0, tail =0, count=0;
int numthreads, numbuffers;

void getargs(int *port, int *num1, int *num2, int argc, char *argv[])
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *num1 = atoi(argv[2]);
    *num2 = atoi(argv[3]);
}

void *thread(void *avg){
    int *buffer = (int *) avg;
    int connfd;
    while(1) {
        pthread_mutex_lock(&mutexT);
        while (count == 0) pthread_cond_wait(&full, &mutexT);

        connfd = buffer[tail];
        tail = (tail + 1) % numbuffers;
        count--;

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutexT);

        requestHandle(connfd);
        Close(connfd);
    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    getargs(&port, &numthreads, &numbuffers, argc, argv);
    int* buffer =(int *) malloc(numbuffers * sizeof(int));
    pthread_mutex_init(&mutexT, NULL);
    pthread_cond_init(&empty, NULL);
    pthread_cond_init(&full, NULL);

    // 
    // CS537: Create some threads...
    //
    pthread_t* p = (pthread_t  *) malloc(numthreads * sizeof(pthread_t));
    int i;
    for(i=0;i<numthreads;i++)
        pthread_create(p+i, NULL, thread, buffer);

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        pthread_mutex_lock(&mutexT);
        while (count == numbuffers) pthread_cond_wait(&empty, &mutexT);

        buffer[head] = connfd;
        head = (head + 1) % numbuffers;
        count++;

        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutexT);

        //
        // CS537: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work. However, for SFF, you may have to do a little work
        // here (e.g., a stat() on the filename) ...
        //
        //requestHandle(connfd);

        //Close(connfd);
    }

}


    


 
