#include <arpa/inet.h>
#include <curses.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int connectToServer ( int *, char[], char[] );

int main ( int argc, char *argv[] ) {
  initscr ( );	      // Initialize the window
  noecho ( );	      // Don't echo any keypresses
  curs_set ( FALSE ); // Don't display a cursor
  clear ( );

  int sd;
  if ( argc != 3 ) {
    mvprintw ( 0, 0, "[client] Sintaxa: %s <adresa_server> <port>\n",
	       argv[ 0 ] );

    refresh ( );
    sleep ( 3 );
    endwin ( );
    return -1;
  }

  if ( connectToServer ( &sd, argv[ 1 ], argv[ 2 ] ) ) {
    return -1;
  } else {
    clear ( );
    mvprintw ( 0, 0, "[client] Conectat la server!\n" );
    refresh ( );
  }
  sleep ( 3 );
  write ( sd, "salut", sizeof ( "salut" ) );
  sleep ( 10 );
}

int connectToServer ( int *sd, char cAddress[], char cPort[] ) {
  struct sockaddr_in server;
  /* stabilim portul */
  int port = atoi ( cPort );

  /* cream socketul */
  if ( ( *sd = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
    mvprintw ( 0, 0, "[client] Eroare la socket().\n" );
    refresh ( );
    sleep ( 3 );
    endwin ( );
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr ( cAddress );
  /* portul de conectare */
  server.sin_port = htons ( port );

  /* ne conectam la server */
  if ( connect ( *sd, ( struct sockaddr * ) &server,
		 sizeof ( struct sockaddr ) ) == -1 ) {
    mvprintw ( 0, 0, "[client] Eroare la connect().\n" );

    refresh ( );
    sleep ( 3 );
    endwin ( );
    perror ( "eroare la connect" );
    return errno;

  } else {
    mvprintw ( 0, 0, "[client] Conectat la server.\n" );
  }
  return 0;
}
