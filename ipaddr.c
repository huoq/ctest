#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

void main() {
	uint32_t ip = 0;
	char ips[20];
	inet_pton(AF_INET, "100.100.100.100", &ip);
	printf("%u\n",ip);
	ip ++;
	ip += 256;
	inet_ntop(AF_INET, (void *)&ip, ips, 16);
	printf("inet_ntop: %s\n", ips);
}
