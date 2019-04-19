#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lib.h"

#define BUFSIZE 4096 // max number of bytes we can get at once

/**
 * Struct to hold all three pieces of a URL
 */
typedef struct urlinfo_t {
  char *hostname;
  char *port;
  char *path;
} urlinfo_t;

/**
 * Tokenize the given URL into hostname, path, and port.
 *
 * url: The input URL to parse.
 *
 * Store hostname, path, and port in a urlinfo_t struct and return the struct.
*/
urlinfo_t *parse_url(char *url)
{
  // copy the input URL so as not to mutate the original
  char *hostname = strdup(url);
  char *port;
  char *path;

  urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));
  char *slash;

  //  We can parse the input URL by doing the following:

  // 1. Use strchr to find the first backslash in the URL (this is assuming there is no http:// or https:// in the URL).
  if (strstr(hostname, "http://") == NULL && strstr(hostname, "https://") == NULL)
  {
    slash = strchr(hostname, (int) '/');
  }
  else
  {
    slash = hostname + strlen(hostname);
  }
  
  // 2. Set the path pointer to 1 character after the spot returned by strchr.
  if (slash)
  {
    path = slash + 1;
  }
  else
  {
    path = NULL;
  }
  
  // 3. Overwrite the backslash with a '\0' so that we are no longer considering anything after the backslash.
  if (slash)
  {
    *slash = '\0';
  }
  // 4. Use strchr to find the first colon in the URL.
  char *colon = strchr(hostname, (int) ':');
  // 5. Set the port pointer to 1 character after the spot returned by strchr.
  if (colon)
  {
    port = colon + 1;
  }
  else
  {
    port = NULL;
  }
  
  // 6. Overwrite the colon with a '\0' so that we are just left with the hostname.
  if (colon)
  {
    *colon = '\0';
  }

  urlinfo->hostname = hostname;
  urlinfo->port = port;
  urlinfo->path = path;

  return urlinfo;
}

/**
 * Constructs and sends an HTTP request
 *
 * fd:       The file descriptor of the connection.
 * hostname: The hostname string.
 * port:     The port string.
 * path:     The path string.
 *
 * Return the value from the send() function.
*/
int send_request(int fd, char *hostname, char *port, char *path)
{
  const int max_request_size = 16384;
  char request[max_request_size];
  int rv;

  int request_size = snprintf(request, max_request_size, "GET /%s HTTP/1.1\nHost: %s:%s\nConnection: close\n\n", path, hostname, port);


  rv = send(fd, request, request_size, 0);

  if (rv < 0)
  {
    perror("send request");
  }

  return 0;
}

int main(int argc, char *argv[])
{  
  int sockfd, numbytes;  
  char buf[BUFSIZE];

  if (argc != 2) {
    fprintf(stderr,"usage: client HOSTNAME:PORT/PATH\n");
    exit(1);
  }
  // 1. Parse the input URL
  urlinfo_t *url_info = parse_url(argv[1]);
  // 2. Initialize a socket by calling the `get_socket` function from lib.c
  sockfd = get_socket(url_info->hostname, url_info->port);
  // 3. Call `send_request` to construct the request and send it
  send_request(sockfd, url_info->hostname, url_info->port, url_info->path);
  // 4. Call `recv` in a loop until there is no more data to receive from the server. Print the received response to stdout.
  while ((numbytes = recv(sockfd, buf, BUFSIZE - 1, 0)) > 0)
  {
    fprintf(stdout, "%s", buf);
  }
  fprintf(stdout, "\n");
  // 5. Clean up any allocated memory and open file descriptors.
  // printf("1\n");
  // free(url_info->hostname);
  // printf("2\n");
  // free(url_info->port);
  // printf("3\n");
  // free(url_info->path);
  // printf("4\n");
  close(sockfd);
  free(url_info);
  // printf("5\n");

  return 0;
}
