#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <cassert>
#include <time.h>
#include <cstdint>
#include <sys/time.h>
#include <unistd.h>

struct results_ttl {
    char ip[3][20];
    struct timeval t_start[3];
    struct timeval t_end[3];
    int answers;
    int echo_replys;
    bool answer_back[3];
    u_int16_t ID[3]; 
    u_int16_t SEQ[3];
};

// traceroute.cpp
int print_answer(int ttl, struct results_ttl * ANSWER);
int traceroute(int sockfd, struct in_addr *ip_address);

// icmp.cpp
u_int16_t compute_icmp_checksum(const u_int16_t *buff, int length);
struct icmp create_icmp_echo_header(u_int16_t id, u_int16_t seq);
int send_icmp_echo(int sockfd, struct icmp *header, struct in_addr *ip_addr, 
                        int ttl, struct timeval *time_start);
int analyse_packet(u_int8_t * buffer, struct sockaddr_in *sender, 
                        struct results_ttl * ANSWER);
int receive_icmp(int sockfd, struct results_ttl * ANSWER);
u_int16_t choose_id();
u_int16_t choose_seq();
int send_and_receive_icmp(int sockfd, int ttl, struct in_addr *ip_addr, 
                            struct results_ttl * ANSWER);

// func.cpp
bool address_check(char * adr);
int timeval_subtract (struct timeval *result, struct timeval *x, 
                        struct timeval *y);


#endif // TRACEROUTE_H