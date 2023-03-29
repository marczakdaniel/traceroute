#include "traceroute.h"

bool address_check(char * adr) {
    int i = 0;
    char buf[6];
    int buf_i = 0;
    while (adr[i] != '\0') {
        if (!((adr[i] <= '9' && adr[i] >= '0') || adr[i] == '.')) {
            return false;
        }
        if (adr[i] == '.') {
            if (buf_i == 0) {
                return false;
            }
            buf[buf_i] = '\0';
            int x = std::stoi(buf);
            if (x > 255) {
                return false;
            }
            if (buf[0] == '0' && x != 0) {
                return false;
            }
            if (buf_i > 1 && buf[0] == '0' && buf[1] == '0') {
                return false;
            }
            buf_i = 0;
        }
        else {
            if (buf_i == 4) {
                return false;
            }
            buf[buf_i] = adr[i];
            buf_i++;
        }
        i++;
    }
    if (buf_i == 0) {
        return false;
    }
    buf[buf_i] = '\0';
    int x = std::stoi(buf);
    if (x > 255) {
        return false;
    }
    if (buf[0] == '0' && x != 0) {
        return false;
    }
    if (buf_i > 1 && buf[0] == '0' && buf[1] == '0') {
        return false;
    }
    buf_i = 0;
    return true;
}

// Code from THE GNU C LIBRARY: 
// https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y) {
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
        tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}