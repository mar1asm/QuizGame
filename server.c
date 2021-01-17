#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct thData {
  int idThread; // id-ul thread-ului tinut in evidenta de acest program
  int cl;	// descriptorul intors de accept
  int next;
} thData;

pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0;

thData *threadsData[ 100 ];

static void *treat ( void * ); /* functia executata de fiecare thread ce
				  realizeaza comunicarea cu clientii */
int raspunde ( void * );

int setupServer ( int *sd ) {

  int port = 2020;
  struct sockaddr_in server; /* structurile pentru server si clienti */

  if ( ( *sd = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
    perror ( "[server] Eroare la socket().\n" );
    return errno;
  }
  int optval = 1; /* optiune folosita pentru setsockopt()*/
  setsockopt ( *sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof ( optval ) );

  bzero ( &server, sizeof ( server ) );

  /* umplem structura folosita de server */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl ( INADDR_ANY );
  server.sin_port = htons ( port );

  /* atasam socketul */
  if ( bind ( *sd, ( struct sockaddr * ) &server,
	      sizeof ( struct sockaddr ) ) == -1 ) {
    perror ( "[server] Eroare la bind().\n" );
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if ( listen ( *sd, 5 ) == -1 ) {
    perror ( "[server] Eroare la listen().\n" );
    return errno;
  }
  printf ( "[server] Serverul a pornit\n" );
  return 0;
}

int main ( ) {
  errno = 0;
  int clientCount = 0;
  int sd;
  struct timeval tv;
  tv.tv_sec = 2; /* se va astepta un timp de 1 sec. */
  tv.tv_usec = 0;

  struct sockaddr_in from;
  pthread_t th[ 100 ]; // Identificatorii thread-urilor care se vor crea
  bzero ( &from, sizeof ( from ) );
  if ( setupServer ( &sd ) ) {
    return -1;
  }
  int len = sizeof ( from );

  for ( int i = 0; i < 10; i++ ) {
    thData *td;

    /* int client = accept ( sd, ( struct sockaddr * ) &from, &len );
    if ( client < 0 ) {
      perror ( "[thread]Eroare la accept().\n" );
      return errno;
    } */
    int client = 2;

    // setsockopt ( client, SOL_SOCKET, SO_RCVTIMEO, ( const char * ) &tv,
    //	 sizeof tv );
    td = ( struct thData * ) malloc ( sizeof ( struct thData ) );
    td->idThread = clientCount;
    td->cl = client;
    td->next = 0;
    threadsData[ clientCount ] = td;

    if ( clientCount != 0 ) {
      threadsData[ clientCount - 1 ]->next = clientCount;
    }

    pthread_create ( &th[ clientCount ], NULL, &treat, td );

    clientCount++;
  }
  sleep ( 30 );
}

static void *treat ( void *arg ) {

  struct thData tdL;
  tdL = *( ( struct thData * ) arg );
  // printf ( "[thread]- %d - Asteptam mesajul...\n", tdL.idThread );
  fflush ( stdout );
  pthread_detach ( pthread_self ( ) );

  pthread_mutex_lock ( &mutex );

  for ( int i = 0; i < 10; i++ ) {
    while ( turn != tdL.idThread )
      pthread_cond_wait ( &cond, &mutex );
    printf ( "Hello from thread %d\n", tdL.idThread );
    sleep ( 1 );
    turn = threadsData[ tdL.idThread ]->next;
    pthread_cond_broadcast ( &cond );
  }
  pthread_mutex_unlock ( &mutex );

  // raspunde ( ( struct thData * ) arg );
  /* am terminat cu acest client, inchidem conexiunea */
  close ( ( intptr_t ) arg );
  return ( NULL );
};

int raspunde ( void *arg ) {

  struct thData tdL;
  tdL = *( ( struct thData * ) arg );

  printf ( "S-a conectat un client!\n" );
  char input[ 20 ];
  int readErr;
  readErr = read ( tdL.cl, input, 6 );

  if ( errno == 0 && readErr > 0 ) {
    printf ( "Clientul a raspuns, mesajul este %s\n", input );
    return 0;
  }

  if ( errno == EAGAIN ) {
    printf ( "Clientul nu a raspuns in timpul acordat\n" );
    return -1;
  }

  perror ( "Clientul s-a deconectat" );
}
