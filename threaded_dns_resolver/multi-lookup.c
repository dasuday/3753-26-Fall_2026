#include "multi-lookup.h"
#include "array.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static int resolve_ipv4_address(const char *hostname, char *ipv4, size_t ipv4_len) {
  struct addrinfo hints;
  struct addrinfo *results = NULL;
  struct addrinfo *current = NULL;
  int status = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, NULL, &hints, &results);
  if (status != 0) {
    return -1;
  }

  for (current = results; current != NULL; current = current->ai_next) {
    struct sockaddr_in *address = (struct sockaddr_in *) current->ai_addr;
    if (inet_ntop(AF_INET, &(address->sin_addr), ipv4, ipv4_len) != NULL) {
      freeaddrinfo(results);
      return 0;
    }
  }

  freeaddrinfo(results);
  return -1;
}

int main() {
  // sample hostname
  const char *hostname = "google.com";

  // buffer to store the first IPv4 address returned by getaddrinfo
  char ipv4[INET_ADDRSTRLEN];

  // resolve hostname to IPv4 address
  if (!resolve_ipv4_address(hostname, ipv4, sizeof(ipv4))) {
    printf("%s -> %s\n", hostname, ipv4);
  } else {
    printf("%s -> NOT_RESOLVED\n", hostname);
  }

  return 0;
}
