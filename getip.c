
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

void getipv4addr(const char *iface) {
  struct ifaddrs *ifaddr, *ifa;
  char ip[INET_ADDRSTRLEN];
  
  if(getifaddrs(&ifaddr) == -1) {
	perror("getifaddrs");
	return;
  }  

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {	
	  if (ifa->ifa_addr == NULL)
	    continue;

	  int family = ifa->ifa_addr->sa_family;
	  if (family == AF_INET && strcmp(ifa->ifa_name, iface) == 0) {

		void *addr = &( (struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
		
		if(inet_ntop(family, addr, ip, sizeof(ip)) == NULL) {
		  perror("inet_ntop");
		  continue;
		}
		printf("%s: %s\n", ifa->ifa_name, ip);
	  }
  }

  freeifaddrs(ifaddr);

}

int main() {
  const char *iface = "wlan0";
  getipv4addr(iface);
  return 0;
}
