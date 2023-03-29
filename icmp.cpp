#include "traceroute.h"

u_int16_t N_SEQ = 1;

/*------------- Send icmp packet -------------*/

u_int16_t compute_icmp_checksum(const u_int16_t *buff, int length) {
	u_int32_t sum;
	const u_int16_t* ptr = buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}

struct icmp create_icmp_echo_header(u_int16_t id, u_int16_t seq) {
    struct icmp header;
    bzero(&header, sizeof(header));
    header.icmp_type = ICMP_ECHO;
    header.icmp_code = 0;
    header.icmp_hun.ih_idseq.icd_id = id;
    header.icmp_hun.ih_idseq.icd_seq = seq;
    header.icmp_cksum = 0;
    header.icmp_cksum = 
        compute_icmp_checksum((u_int16_t *)&header, sizeof(header));

    return header;
}

int send_icmp_echo(int sockfd, struct icmp *header, struct in_addr *ip_addr, 
                    int ttl, struct timeval *time_start) {
    // wpisujemy adres odbiorcy do struktury adresowej
    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    recipient.sin_addr = *ip_addr;

    // konfigurowanie
    int ttl_ip = ttl;
    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl_ip, sizeof(int)) < 0) {
        fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // wysylanie pakietu przez gniazdo
    gettimeofday(time_start, 0);
    ssize_t bytes_sent = sendto (
        sockfd,
        header,
        sizeof(header),
        0,
        (struct sockaddr *)&recipient,
        sizeof(recipient)
    );
    if (bytes_sent == -1) {
        fprintf(stderr, "sendto error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*------------- Receive icmp packet -------------*/

int analyse_packet(u_int8_t * buffer, struct sockaddr_in *sender, struct results_ttl * ANSWER) {
    char ip_str[20];
    inet_ntop(AF_INET, &(sender->sin_addr), ip_str, sizeof(ip_str));
    if (ip_str == NULL) {
        fprintf(stderr, "inet_ntop error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct ip *ip_header = (struct ip *) buffer;
    u_int8_t *icmp_packet = buffer + 4 * ip_header->ip_hl;
    struct icmp *icmp_header = (struct icmp *)icmp_packet;

    u_int8_t icmp_type = icmp_header->icmp_type;

    if (icmp_type == ICMP_ECHOREPLY) {
        u_int16_t id_icmp = icmp_header->icmp_hun.ih_idseq.icd_id;
        u_int16_t seq_icmp = icmp_header->icmp_hun.ih_idseq.icd_seq;
        for (int i = 0; i < 3; i++) {
            if (id_icmp == ANSWER->ID[i] && seq_icmp == ANSWER->SEQ[i]) {
                gettimeofday(&(ANSWER->t_end[i]), 0);
                strncpy(ANSWER->ip[i], ip_str, 20);
                ANSWER->answers++;
                ANSWER->echo_replys++;
                ANSWER->answer_back[i] = true;
            }
        }
    }
    else if (icmp_type == ICMP_TIME_EXCEEDED) {
        u_int8_t * buffer2 = icmp_packet + 8;
        struct ip *ip_header2 = (struct ip *) buffer2;
        u_int8_t *icmp_packet2 = buffer2 + 4 * ip_header2->ip_hl;
        struct icmp *icmp_header2 = (struct icmp *)icmp_packet2;

        u_int16_t id_icmp = icmp_header2->icmp_hun.ih_idseq.icd_id;
        u_int16_t seq_icmp = icmp_header2->icmp_hun.ih_idseq.icd_seq;
        for (int i = 0; i < 3; i++) {
            if (id_icmp == ANSWER->ID[i] && seq_icmp == ANSWER->SEQ[i]) {
                gettimeofday(&(ANSWER->t_end[i]), 0);
                strncpy(ANSWER->ip[i], ip_str, 20);
                ANSWER->answers++;
                ANSWER->answer_back[i] = true;
            }
        }
    }
    return EXIT_SUCCESS;
}

int receive_icmp(int sockfd, struct results_ttl * ANSWER) {
    fd_set descriptors;
    FD_ZERO (&descriptors);
    FD_SET (sockfd, &descriptors);
    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
    struct timeval begin, end;
    int ready = 1;
    ssize_t packet_len = 0;
    gettimeofday(&begin, 0);
    while (1) {
        ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);
        if (ready < 0) {
            fprintf(stderr, "select error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        if (ready == 0) {
            break;
        }
        do {
            struct sockaddr_in sender;
            socklen_t sender_len = sizeof(sender);
            u_int8_t buffer[IP_MAXPACKET];

            packet_len = recvfrom (
                sockfd,
                buffer,
                IP_MAXPACKET,
                MSG_DONTWAIT,
                (struct sockaddr *)&sender, 
                &sender_len
            );
            if (packet_len == -1) {
                if (errno == EWOULDBLOCK) { // normal error
                    break;
                }
                else {
                    fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
                    return EXIT_FAILURE;
                }
            }
            if (packet_len == 0) {
                fprintf(stderr, "recvfrom: connection closed\n");
                return EXIT_FAILURE;
            }

            if (analyse_packet(buffer, &sender, ANSWER) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }
            if (ANSWER->answers == 3) {
                break;
            }
        } while (packet_len);
        if (ANSWER->answers == 3) {
            break;
        }
        gettimeofday(&end, 0);
        struct timeval result; 
        timeval_subtract(&result, &end, &begin);
        struct timeval tv_new; 
        if (timeval_subtract(&tv_new, &tv, &result) == 1) {
            break;
        }
        tv = tv_new;
    }
    return EXIT_SUCCESS;
}

/*------------- Send and receive icmp packet -------------*/

u_int16_t choose_id() {
    u_int16_t x = getpid();
    return x;
}
u_int16_t choose_seq() {
    u_int16_t x = htons(N_SEQ);
    N_SEQ += 1;
    return x;
}

int send_and_receive_icmp(int sockfd, int ttl, struct in_addr *ip_addr, struct results_ttl * ANSWER) {
    for (int i = 0; i < 3; i++) {
        ANSWER->ID[i] = choose_id();
        ANSWER->SEQ[i] = choose_seq();
    }
    for (int i = 0; i < 3; i++) {
        struct icmp header = create_icmp_echo_header(ANSWER->ID[i], ANSWER->SEQ[i]);
        int send_complete = send_icmp_echo(sockfd, &header, ip_addr, ttl, &(ANSWER->t_start[i]));
        if (send_complete == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }
    int receive_complete = receive_icmp(sockfd, ANSWER);
    return receive_complete;
}