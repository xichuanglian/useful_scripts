#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT 12345
#define BUF_SIZE 10
#define TIMEOUT_MS 10
#define RUN_CNT 10

void usage()
{
    printf("Usage:\n\tServer : cdiff -s\n\tClient : cdiff -c hosts\n");
}

void server()
{
    struct sockaddr_in localaddr;
    struct sockaddr_in remaddr;
    int skt;
    uint32_t recvlen;
    uint32_t addrlen;
    uint32_t ip;
    char buf[BUF_SIZE];
    struct timeval tv;
    uint64_t t_usec;

    if ((skt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Failed to create socket.\n");
        return;
    }

    memset((void*)&localaddr, 0, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localaddr.sin_port = htons(PORT);

    if (bind(skt, (struct sockaddr *)&localaddr, sizeof(localaddr)) < 0) {
        perror("Failed to bind socket\n");
        return;
    }
    printf("Waiting on port %d\n", PORT);
    while (1) {
        recvlen = recvfrom(skt, buf, BUF_SIZE, 0, (struct sockaddr*)&remaddr, &addrlen);
        gettimeofday(&tv, NULL);
        t_usec = 1e6 * tv.tv_sec + tv.tv_usec;
        memcpy(buf, &t_usec, sizeof(t_usec));
        sendto(skt, buf, sizeof(t_usec), 0, (struct sockaddr*)&remaddr, addrlen);
        ip = ntohl(remaddr.sin_addr.s_addr);
        printf("%d.%d.%d.%d - %ldus\n",
               (ip >> 24) & 0xff,
               (ip >> 16) & 0xff,
               (ip >>  8) & 0xff,
               ip & 0xff, t_usec);
    }
}

int calc_diff(uint32_t addr, char* ip)
{
    struct sockaddr_in servaddr;
    int skt;
    int i;
    int diff[RUN_CNT];
    int rtt[RUN_CNT];
    int failed;
    char buf[BUF_SIZE];
    int recvlen;
    struct timeval tv;
    uint64_t st, ed, rply;
    int max_rtt, m_idx;
    int s_diff;
    
    memset((char*)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family= AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(addr);

    if ((skt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Failed to create socket.\n");
        return 0;
    }
    tv.tv_sec = 0;
    tv.tv_usec = TIMEOUT_MS * 1e3;
    if (setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Faied to set recv timeout\n");
        return 0;
    }

    for (i = 0; i < RUN_CNT; ++i) {
        failed = 1;
        while (failed) {
            gettimeofday(&tv, NULL);
            st = tv.tv_sec * 1e6 + tv.tv_usec;
            if (sendto(skt, buf, 1, 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
                fprintf(stderr, "Failed to send request to %s\n", ip);
                failed = 1;
                sleep(1);
            } else {
                recvlen = recv(skt, buf, BUF_SIZE, 0);
                if (recvlen < 0) {
                    fprintf(stderr, "Wait for reply timed out: %s\n", ip);
                    failed = 1;
                    sleep(1);
                } else {
                    gettimeofday(&tv, NULL);
                    ed = tv.tv_sec * 1e6 + tv.tv_usec;
                    memcpy(&rply, buf, sizeof(rply));
                    failed = 0;
                }
            }
        }
        diff[i] = rply - st;
        rtt[i] = ed - st;
    }

    m_idx = max_rtt = 0;
    for (i = 0; i < RUN_CNT; ++i) {
        if (rtt[i] > max_rtt) {
            max_rtt = rtt[i];
            m_idx = i;
        }
    }

    diff[m_idx] = 0;
    rtt[m_idx] = 0;
    s_diff = 0;
    for (i = 0; i < RUN_CNT; ++i) {
        s_diff += diff[i] - rtt[i]/2;
    }

    return s_diff / (RUN_CNT - 1);
}

void client(char* fname)
{
    char line[256];
    FILE* file;
    uint8_t ip[4];
    uint32_t ipaddr;
    int diff;
    int i;
    
    file = fopen(fname, "r");
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%hhu.%hhu.%hhu.%hhu", &ip[3], &ip[2], &ip[1], &ip[0]);
        ipaddr = 0;
        for (i = 0; i < 4; ++i) {
            ipaddr += ip[i] << (8 * i);
        }
        diff = calc_diff(ipaddr, line);
        line[strlen(line) - 1] = ' ';
        printf("%s%d\n", line, diff);
    }
    
    fclose(file);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        usage();
        goto exit;
    }

    if (strcmp("-s", argv[1]) == 0) {
        server();
    } else if (strcmp("-c", argv[1]) == 0) {
        if (argc < 3) {
            usage();
            goto exit;
        } else {
            client(argv[2]);
        }
    } else {
        usage();
        goto exit;
    }
    
 exit:
    return 0;
}
