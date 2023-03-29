#include "traceroute.h"

int print_answer(int ttl, struct results_ttl * ANSWER) {
    printf("%d. ", ttl);
    int time_average = 0;
    if (ANSWER->answers == 0) {
        printf("*\n");
        return 0;
    }
    for (int i = 0; i < 3; i++) {
        if (ANSWER->answer_back[i]) {
            if (i == 0) {
                printf("%s ", ANSWER->ip[i]);
            }
            else if (i == 1 && 
                    (strcmp(ANSWER->ip[0], ANSWER->ip[1]) || !ANSWER->answer_back[0])) {
                printf("%s ", ANSWER->ip[1]);
            }
            else if (i == 2 && 
                    ((strcmp(ANSWER->ip[0], ANSWER->ip[2]) || !ANSWER->answer_back[0]) && 
                    (strcmp(ANSWER->ip[1], ANSWER->ip[2]) || !ANSWER->answer_back[1]))) {
                printf("%s ", ANSWER->ip[2]);
            }
            struct timeval result_time;
            timeval_subtract(&result_time, &(ANSWER->t_end[i]), &(ANSWER->t_start[i]));
            time_average += result_time.tv_sec * 1000000 + result_time.tv_usec;
        }
    }
    if (ANSWER->answers < 3) {
        printf("???");
    }
    else {
        time_average /= 3000;
        printf("%dms", time_average);
    }
    printf("\n");
    if (ANSWER->echo_replys == 3) {
        return 1;
    }
    return 0;
}

int traceroute(int sockfd, struct in_addr *ip_address) {
    for (int ttl = 1; ttl <= 30; ttl++) {
        struct results_ttl ANSWER;
        bzero(&ANSWER, sizeof(ANSWER));
        int icmp_complete = send_and_receive_icmp(sockfd, ttl, ip_address, &ANSWER); 
        if (icmp_complete == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }

        int found_ip = print_answer(ttl, &ANSWER);
        if (found_ip == 1) {
            return EXIT_SUCCESS;
        }
    } 
    return EXIT_SUCCESS;
}