#include "traceroute.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "invalid input\n");
        return EXIT_FAILURE;
    }
    struct in_addr ip_address;
    if (inet_pton(AF_INET, argv[1], &ip_address) <= 0) {
        fprintf(stderr, "invalid input\n");
        return EXIT_FAILURE;
    }
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}
    int trace_complete = traceroute(sockfd, &ip_address);
    return trace_complete;
}