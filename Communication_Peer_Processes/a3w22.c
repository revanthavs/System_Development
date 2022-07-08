#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include <poll.h>

#define MAX_nSW 7
#define MAXWORD 32
#define MIN_IPaddress 0
#define MAX_IPaddress 1000
#define MAXBUF 80

/*
    Name: Revanth Atmakuri
    Student ID: 1684293
    CCID: ATMAKURI
    CMPUT 379, Winter 2022
*/


int debug = 1; // 1: Run debug messages 0: No debug messages

struct psw
{
    int psw_num; int port1; int port2;
    int port3l; int port3h;
};

struct forwarding_table
{
    int scrIPl; int scrIPh; int destIPl;
    int destIPh; int action;
    int action_des; int pktCount;
};

// Added arg5
struct message
{
    // char* name;
    int name;
    int arg1; int arg2;
    int arg3; int arg4; int arg5; int action;
    int fwd; int pktCount;
};

void printSwitch(struct forwarding_table frw_table[], int f_t_i, int ADMIT, int HELLO_ACK, int ADD, int RELAYIN, int HELLO, int ASK, int RELAYOUT, struct psw this_switch){
    printf("Forwarding table\n");
    for (int i = 0; i < f_t_i; i++){
        char * act = "DROP\0";
        if (frw_table[i].action) act = "FORWARD\0";
        printf("[%d]  (srcIP= %d-%d, destIP= %d-%d, action= %s:%d, pktCount= %d)\n", this_switch.psw_num, frw_table[i].scrIPl, frw_table[i].scrIPh, frw_table[i].destIPl, frw_table[i].destIPh, act, frw_table[i].action_des, frw_table[i].pktCount); 
    }
    printf("Packet Stats:\n       Received:     ADMIT:%d, HELLO_ACK:%d, ADD:%d, RELAYIN:%d\n      Transmitted: HELLO:%d, ASK:%d, RELAYOUT:%d\n", ADMIT, HELLO_ACK, ADD, RELAYIN, HELLO, ASK, RELAYOUT);
    return;
}

/*
    returns -2 if destIP in table and its a DROP
    returns table_number if destIP in table and its a FORWARD
    returns -1 if destIP in not in table
*/
int ForwardingTableCheck(struct forwarding_table frw_table[], int f_t_i, int srcIP, int destIP){
    for (int i = 0; i < f_t_i; i++){
        char drop[] = "DROP\0", forward[] = "FORWARD\0";
        if (frw_table[i].destIPl <= destIP && destIP <= frw_table[i].destIPh){
            if (frw_table[i].action == 0){
                frw_table[i].pktCount++;
                return -2;
            }
            if (frw_table[i].action == 1){
                frw_table[i].pktCount++;
                return i;
            }
        }
    }
    return -1;
}

/*
    This function acts as a client aka PacketSwitch
    It returns -1 if Packet Switch had any issue while running
    It returns 1 if Packet Switch exits normally when user enters exit in command-line
*/

int PacketSwitch(int pswi, char* filename, int pswj, int pswk, int IPlow, int IPhigh, char* server_addr, int portNumber){
    int len, sfd, rval = 0, timeOut = 0, N = 2;
    char servername[MAXWORD], buf[] = "From client", buff[MAXBUF];

    int ADMIT = 0, HELLO_ACK = 0, ADD = 0, RELAYIN = 0, HELLO = 0, ASK = 0, RELAYOUT = 0, f_t_i = 0;
    struct forwarding_table frw_table[100];
    struct psw this_switch;
    this_switch.psw_num = pswi; this_switch.port1 = pswj; this_switch.port2 = pswk; this_switch.port3l = IPlow;
    this_switch.port3h = IPhigh;
    
    struct pollfd pfd[N];

    // Opening a file to read
    FILE* input_file = fopen(filename, "r");
    if (input_file == NULL) printf("File failed to open\n");
    char single_line[MAXBUF];
    memset((char *) &single_line, 0, sizeof(single_line));

    strcpy(servername, server_addr);
    struct sockaddr_in server;
    struct hostent *hp;
    hp = gethostbyname(servername);
    if (hp == (struct hostent *) NULL) {printf("Client connection failed\n"); return -1;}
    memset((char *) &server, 0, sizeof server);
    memcpy((char *) &server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_family = AF_INET; server.sin_port = htons(portNumber);

    if ((sfd= socket(AF_INET, SOCK_STREAM, 0)) < 0){printf("Failed to create a client socket\n"); return -1;}

    // For I/O Polling
    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;

    // For Master Switch & Packet Switch polling
    pfd[1].fd = sfd;
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;

    // Blocking System call
    if (connect(sfd, (struct sockaddr *) &server, sizeof(server)) < 0){printf("Failed to connect\n"); return -1;}
    // Test write to server
    if (debug) printf("After client connect\n");
    // len = write(sfd, buf, sizeof(buf));
    memset(&buff, 0, sizeof(buff));
    if (debug){ len = read(sfd, buff, sizeof(buff)); printf("From server: %s", buff);}
    struct message hello_msg;
    // char hello[] = "HELLO";
    // "HELLO" <==> 10
    hello_msg.name = 10;
    hello_msg.arg1 = pswi;
    hello_msg.arg2 = pswj;
    hello_msg.arg3 = pswk;
    hello_msg.arg4 = IPlow;
    hello_msg.arg5 = IPhigh;
    len = write(sfd, (char *) &hello_msg, sizeof(hello_msg));
    printf("Transmitted (src= psw%d, dest= master) [HELLO]:\n         (port0= master, port1= %d, port2= %d, port3= %d-%d\n", hello_msg.arg1, hello_msg.arg2, hello_msg.arg3, hello_msg.arg4, hello_msg.arg5);
    HELLO++;
    memset((char *) &hello_msg, 0, sizeof(hello_msg));
    len = read(sfd, (char *) &hello_msg, sizeof(hello_msg));
    HELLO_ACK++;
    printf("Received (src= master, dest= psw%d) [HELLO_ACK]\n", pswi);
    // Initial Forwarding table
    frw_table[f_t_i].scrIPl = MIN_IPaddress; frw_table[f_t_i].scrIPh = MAX_IPaddress; frw_table[f_t_i].destIPl = IPlow;
    frw_table[f_t_i].destIPh = IPhigh; frw_table[f_t_i].action = 1; frw_table[f_t_i].action_des = 3; frw_table[f_t_i].pktCount = 0;
    f_t_i++;

    while(13){
        rval = poll(pfd, N, timeOut);
        char buf[MAXBUF];
        memset(buff, 0, MAXBUF);
        memset((char *) &single_line, 0, sizeof(single_line));
        // I/O Check
        if (pfd[0].revents & POLLIN){
            memset(buff, 0, MAXBUF);
            len = read(pfd[0].fd, buff, MAXBUF);
            if (debug) printf("Inside I/O poll in Switch\n");
            if (len > 0){
                if (debug) printf("Entered Values: %s\n", buff);
                if (buff[0] == 'e' && buff[1] == 'x' && buff[2] == 'i' && buff[3] == 't'){
                    printf("Exiting from Packet Switch\n");
                    break;
                }
                if (buff[0] == 'i' && buff[1] == 'n' && buff[2] == 'f' && buff[3] == 'o'){
                    printSwitch(frw_table, f_t_i, ADMIT, HELLO_ACK, ADD, RELAYIN, HELLO, ASK, RELAYOUT, this_switch);
                    continue;
                }
            }
        }
        // Master Switch Connection
        if (pfd[1].revents & POLLIN){
            struct message curr_msg;
            if (recv(pfd[1].fd, buff, sizeof(buff), MSG_PEEK | MSG_DONTWAIT) == 0){
                printf("lost connection to Master Switch\n");
                if (debug) printf("Master: has been closed\n");
                break;
            }
            memset((char *) &curr_msg, 0, sizeof(curr_msg));
            len = read(pfd[1].fd, (char *) &curr_msg, sizeof(curr_msg));
            if (len > 0){
                // ADD: 13
                if (curr_msg.name == 13){
                    ADD++;
                    char * act = "DROP\0";
                    if (curr_msg.action) act = "FORWARD\0";
                    if (curr_msg.action) RELAYOUT++;
                    printf("Received (src= master, dest= psw%d) [ADD]:\n         (srcIP= %d-%d, destIP= %d-%d, action= %s:%d, pktCount= %d\n", this_switch.psw_num, curr_msg.arg1, curr_msg.arg2, curr_msg.arg3, curr_msg.arg4, act, curr_msg.fwd, curr_msg.pktCount);
                    frw_table[f_t_i].scrIPl = curr_msg.arg1; frw_table[f_t_i].scrIPh = curr_msg.arg2; frw_table[f_t_i].destIPl = curr_msg.arg3;
                    frw_table[f_t_i].destIPh = curr_msg.arg4; frw_table[f_t_i].action = curr_msg.action; frw_table[f_t_i].action_des = curr_msg.fwd;
                    frw_table[f_t_i].pktCount = curr_msg.pktCount; f_t_i++;
                }
            }
        }

        if (fgets(single_line, sizeof(single_line), input_file)){
            if (debug) printf("%s\n", single_line);
            if (single_line[0] == '#') continue;
            if (single_line[0] == 'p'){
                char nu[2]; nu[0] = single_line[3]; nu[1] = '\0';
                int number = 0;
                sscanf(nu, "%d", &number);
                if (debug) printf("Number: %d\n", number);
                if (number != this_switch.psw_num) continue;
                
                if (single_line[5] == 'd') continue;

                char nu1[4], nu2[4]; int num1 = 0, num2 = 0;
                nu1[0] = single_line[5]; nu1[1] = single_line[6]; nu1[2] = single_line[7]; nu1[3] = '\0';
                nu2[0] = single_line[9]; nu2[1] = single_line[10]; nu2[2] = single_line[10]; nu2[3] = '\0';

                sscanf(nu1, "%d", &num1);
                sscanf(nu2, "%d", &num2);

                if (debug) printf("num1: %d num2: %d\n", num1, num2);

                int res = ForwardingTableCheck(frw_table, f_t_i, num1, num2);
                if (res == -1){
                    struct message curr_msg;
                    // memset((char *) &curr_msg, 0, sizeof(curr_msg));
                    // ASK: 12
                    curr_msg.name = 12;
                    curr_msg.arg1 = this_switch.psw_num;
                    curr_msg.arg2 = num1;
                    curr_msg.arg3 = num2;
                    len = write(pfd[1].fd, (char *) &curr_msg, sizeof(curr_msg));
                    ADD++;
                    printf("Transmitted (src= psw%d, dest= master)  [ASK]: header= (srcIP= %d, destIP= %d)\n", this_switch.psw_num, num1, num2);
                    continue;
                }
                // int ind = f_t_i - 1;
                // frw_table[ind].pktCount++;
            }
            memset((char *) &single_line, 0, sizeof(single_line));
        }

    }

    // len = write(sfd, buf, sizeof(buf));
    fclose(input_file);
    close(sfd);
    return 1;
}

void printInfo(struct psw torswitches[], int torswitches_size, int HELLO, int ASK, int HELLO_ACK, int ADD){
    printf("Switch information:\n");
    for (int i = 0; i < torswitches_size; i++){
        printf("[psw%d] port1= %d, port2= %d, port3= %d-%d\n", torswitches[i].psw_num, torswitches[i].port1, torswitches[i].port2, torswitches[i].port3l, torswitches[i].port3h);
    }
    printf("\nPacket Stats:\n      Received:    HELLO:%d, ASK:%d\n      Transmitted: HELLO_ACK:%d, ADD: %d\n", HELLO, ASK, HELLO_ACK, ADD);
    return;
}

/*
    This function acts as server aka Master Switch which can hold upto nSwitchs= 7
    It returns -1 if there is any problem while running
    It reutrns 1 if it exits normally when user enters exit in stdin
*/

int MasterSwitch(int nSwitch, int portNumber){

    int rval, len, timeout = 0, serverfd, newsock[nSwitch], N = 0;
    unsigned int fromlen;
    struct pollfd pfd[nSwitch+2];
    struct sockaddr_in sin, from;
    char buf[MAXBUF];
    // TODO: Need to check the value
    FILE *sfp[nSwitch];

    int torswitches_index = 0, HELLO = 0, ASK = 0, HELLO_ACK = 0, ADD = 0;
    struct psw torswitchs[10];

    // Creating a managing socket

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0){
        perror("socket failed");
        // exit(EXIT_FAILURE);
        return -1;
    }

    // Binding 

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(portNumber);

    if (bind(serverfd, (struct sockaddr *) &sin, sizeof sin) < 0){
        perror("bind failed");
        // exit(EXIT_FAILURE);
        return -1;
    }

    // Listening (Indicating how many packet switchs to be expected)

    listen(serverfd, nSwitch);

    // Non-blocking polling from master switch

    // For I/O Polling
    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;

    // For Master Switch
    pfd[1].fd = serverfd;
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;

    while (13){
        // if (debug) printf("Check1\n");
        rval = poll(pfd, N+2, timeout);

        memset(buf, 0, MAXBUF);
        // I/O Check
        if (pfd[0].revents & POLLIN){
            len = read(pfd[0].fd, buf, MAXBUF);
            if (debug) printf("Inside I/O poll\n");
            if (len > 0){
                if (debug) printf("Entered Values: %s\n", buf);
                char exit[] = "exit\0";
                if (buf[0] == 'e' && buf[1] == 'x' && buf[2] == 'i' && buf[3] == 't'){
                    printf("Exiting from Master Switch\n");
                    break;
                    close(serverfd);
                    close(pfd[0].fd);
                    for (int i = 0; i < N; i++) close(newsock[i]);
                    return 1; 
                }
                if (buf[0] == 'i' && buf[1] == 'n' && buf[2] == 'f' && buf[3] == 'o'){
                    printInfo(torswitchs, torswitches_index, HELLO, ASK, HELLO_ACK, ADD);
                    continue;
                }
            }
        }

        // Master switch Check
        if (pfd[1].revents & POLLIN){
            fromlen = sizeof(from);
            if (debug) printf("Adding new client to server\n");
            
            newsock[N] = accept (serverfd, (struct sockaddr *) &from, &fromlen);

            if ((sfp[N] = fdopen(newsock[N],"r+")) < 0){
                perror("failed to accept connection");
                // TODO: Need to close all file destripters
                return -1;
            }

            if (debug) len = write(newsock[N], "Hi from server\n", sizeof("Hi from server\n"));

            pfd[N+2].fd = newsock[N];
            pfd[N+2].events = POLLIN;
            pfd[N+2].revents = 0;
            N++;
        }

        for (int i = 2; i < N+2; i++){
            if (pfd[i].revents & POLLIN){
                struct message curr_msg;
                memset(buf, 0, sizeof(buf));
                // if (debug) printf("Inside a socket connection\n");
                // Check data socket
                if (recv(pfd[i].fd, buf, sizeof(buf), MSG_PEEK | MSG_DONTWAIT) == 0){
                    printf("lost connection to psw%d\n", torswitchs[i-2].psw_num);
                    if (debug) printf("Client: %d has been closed\n", i);
                    N--;
                }
                // if (fgets(buf, MAXBUF, sfp[i-2]) != NULL) printf("Name: %s, psw%d, psw%d, psw%d, %d-%d\n",);//printf("%s\n", buf);
                memset((char *) &curr_msg, 0, sizeof(curr_msg));
                len = read(pfd[i].fd, (char *) &curr_msg, sizeof(curr_msg));
                // printf("%d\n", len);
                if (len > 0){
                    // curr_msg.name[0] == 'H' && curr_msg.name[1] == 'E' && curr_msg.name[2] == 'L' && curr_msg.name[3] == 'L' && curr_msg.name[4] == 'O'
                    // HELLO & HELLO_ACK Communications
                    if (curr_msg.name == 10){
                        torswitchs[torswitches_index].psw_num = curr_msg.arg1; torswitchs[torswitches_index].port1 = curr_msg.arg2;
                        torswitchs[torswitches_index].port2 = curr_msg.arg3; torswitchs[torswitches_index].port3l = curr_msg.arg4;
                        torswitchs[torswitches_index].port3h = curr_msg.arg5; torswitches_index++;
                        HELLO++;
                        if (debug) printf("Name: %s, psw%d, psw%d, psw%d, %d-%d\n", "HELLO\0", curr_msg.arg1, curr_msg.arg2, curr_msg.arg3, curr_msg.arg4, curr_msg.arg5);
                        printf("Received (src= psw%d, dest= master) [HELLO]:\n         (port0= master, port1= %d, port2= %d, port3= %d-%d\n", curr_msg.arg1, curr_msg.arg2, curr_msg.arg3, curr_msg.arg4, curr_msg.arg5);
                        struct message reply;
                        // reply.name = "HELLO_ACK\0";
                        reply.name = 11;
                        len = write(pfd[i].fd, (char *) &reply, sizeof(reply));
                        HELLO_ACK++;
                        if (debug) printf("HELLO_ACK from server\n");
                        printf("Transmitted (src= master, dest= psw%d) [HELLO_ACK]\n", i);
                    }
                    // ASK & ADD Communication
                    if (curr_msg.name == 12){
                        ASK++; ADD++;
                        printf("Received (src= psw%d, dest= master) [ASK]:  header= (scrIP= %d, destIP= %d)\n", curr_msg.arg1, curr_msg.arg2, curr_msg.arg3);
                        struct message add_msg;
                        memset((char *) &add_msg, 0, sizeof(add_msg));
                        add_msg.name = 13;
                        add_msg.arg1 = MIN_IPaddress;
                        add_msg.arg2 = MAX_IPaddress;
                        int psi = 0, port1 = -1, port2 = -1, checker = -1;
                        for (int j = 0; j < torswitches_index; j++){
                            if (curr_msg.arg1 == torswitchs[j].psw_num){
                                psi = j;
                                checker = 1;
                            }
                        }
                        if (checker == -1 || (torswitchs[psi].port1 == -1 && torswitchs[psi].port2 == -1)){
                            add_msg.arg3 = curr_msg.arg3;
                            add_msg.arg4 = curr_msg.arg3;
                            add_msg.action = 0;
                            add_msg.fwd = 0;
                            add_msg.pktCount = 0;
                            len = write(pfd[i].fd, (char *) &add_msg, sizeof(add_msg));
                            printf("Transmitted (src= master, dest= psw%d) [ADD]\n  (srcIP= %d-%d, destIP=%d-%d, action= DROP:%d, pktCount= 0)\n", i, MIN_IPaddress, MAX_IPaddress,add_msg.arg3, add_msg.arg4, add_msg.fwd);
                            continue;
                        }
                        if (torswitchs[psi].port1 != -1){
                            port1 = torswitchs[psi].port1; int checker = -1;
                            for (int j = 0; j < torswitches_index; j++){
                                if (port1 == torswitchs[j].psw_num){
                                    port1 = j; checker = 1;
                                }
                            }
                            if (checker == -1){
                                add_msg.arg3 = curr_msg.arg3;
                                add_msg.arg4 = curr_msg.arg3;
                                add_msg.action = 0;
                                add_msg.fwd = 0;
                                add_msg.pktCount = 0;
                                len = write(pfd[i].fd, (char *) &add_msg, sizeof(add_msg));
                                printf("Transmitted (src= master, dest= psw%d) [ADD]\n  (srcIP= %d-%d, destIP=%d-%d, action= DROP:%d, pktCount= 0\n)", i, MIN_IPaddress, MAX_IPaddress,add_msg.arg3, add_msg.arg4, add_msg.fwd);
                                continue;
                            }
                            if (torswitchs[port1].port3l <= curr_msg.arg2 && curr_msg.arg2 <= torswitchs[port1].port3h){
                                add_msg.arg3 = torswitchs[port1].port3l;
                                add_msg.arg4 = torswitchs[port1].port3h;
                                add_msg.action = 1;
                                add_msg.fwd = torswitchs[psi].port1;
                                add_msg.pktCount = 0;
                                
                                len = write(pfd[i].fd, (char *) &add_msg, sizeof(add_msg));
                                printf("Transmitted (src= master, dest= psw%d) [ADD]\n  (srcIP= %d-%d, destIP=%d-%d, action= FORWARD:%d, pktCount= 0\n)", i, MIN_IPaddress, MAX_IPaddress,add_msg.arg3, add_msg.arg4, add_msg.fwd);
                                continue;
                            }else{
                                add_msg.arg3 = curr_msg.arg3;
                                add_msg.arg4 = curr_msg.arg3;
                                add_msg.action = 0;
                                add_msg.fwd = 0;
                                add_msg.pktCount = 0;
                                len = write(pfd[i].fd, (char *) &add_msg, sizeof(add_msg));
                                printf("Transmitted (src= master, dest= psw%d) [ADD]\n  (srcIP= %d-%d, destIP=%d-%d, action= DROP:%d, pktCount= 0\n)", i, MIN_IPaddress, MAX_IPaddress,add_msg.arg3, add_msg.arg4, add_msg.fwd);
                                continue;
                            }
                        }
                        
                        if (torswitchs[psi].port2 != -1){
                            port2 = torswitchs[psi].port2; int checker = -1;
                            for (int j = 0; j < torswitches_index; j++){
                                if (port2 == torswitchs[j].psw_num){
                                    port2 = j; checker = 1;
                                }
                            }
                            if (checker == -1){
                                add_msg.arg3 = curr_msg.arg3;
                                add_msg.arg4 = curr_msg.arg3;
                                add_msg.action = 0;
                                add_msg.fwd = 0;
                                add_msg.pktCount = 0;
                                len = write(pfd[i].fd, (char *) &add_msg, sizeof(add_msg));
                                printf("Transmitted (src= master, dest= psw%d) [ADD]\n  (srcIP= %d-%d, destIP=%d-%d, action= DROP:%d, pktCount= 0\n)", i, MIN_IPaddress, MAX_IPaddress,add_msg.arg3, add_msg.arg4, add_msg.fwd);
                                continue;
                            }
                            if (torswitchs[port2].port3l <= curr_msg.arg2 && curr_msg.arg2 <= torswitchs[port2].port3h){
                                add_msg.arg3 = torswitchs[port2].port3l;
                                add_msg.arg4 = torswitchs[port2].port3h;
                                add_msg.action = 1;
                                add_msg.fwd = torswitchs[psi].port2;
                                add_msg.pktCount = 0;
                                len = write(pfd[i].fd, (char *) &add_msg, sizeof(add_msg));
                                printf("Transmitted (src= master, dest= psw%d) [ADD]\n  (srcIP= %d-%d, destIP=%d-%d, action= FORWARD:%d, pktCount= 0\n)", i, MIN_IPaddress, MAX_IPaddress,add_msg.arg3, add_msg.arg4, add_msg.fwd);
                                continue;
                            }else{
                                add_msg.arg3 = curr_msg.arg3;
                                add_msg.arg4 = curr_msg.arg3;
                                add_msg.action = 0;
                                add_msg.fwd = 0;
                                add_msg.pktCount = 0;
                                len = write(pfd[i].fd, (char *) &add_msg, sizeof(add_msg));
                                printf("Transmitted (src= master, dest= psw%d) [ADD]\n  (srcIP= %d-%d, destIP=%d-%d, action= DROP:%d, pktCount= 0\n)", i, MIN_IPaddress, MAX_IPaddress,add_msg.arg3, add_msg.arg4, add_msg.fwd);
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }

    close(serverfd); close(pfd[0].fd);
    for (int i = 0; i < N; i++) close(newsock[i]);
    return 1;
}

void WrongInput(){
    printf("Usage: ./a3w22 master nSwitch portNumber\n");
    printf("       ./a3w22 pswi dataFile (null|pswj) (null|pswk) IPlow-IPhigh serverAddress portNumber\n");
}

int main(int argc, char** argv) {

    // TODO: Need to set limit for CPU

    if (argc != 4 || argc != 8){
        if (argc != 4 && argc != 8){ WrongInput(); return 0; }
    }

    if (debug){
        printf("Command line arguments:\n");
        for (int i = 0; i < argc; i++)
            printf("%s ", argv[i]);
        printf("\n");
    }

    char master[] = "master\0";

    // TODO: Need to check all arguments
    if (argc == 4){
        if (strcmp("master\0", argv[1]) != 0){ WrongInput(); return 0;}
        // Default portNumber
        int portNumber = 9293, nSwitch = 0;
        sscanf(argv[2], "%d", &nSwitch);
        sscanf(argv[3], "%d", &portNumber);

        if (MasterSwitch(nSwitch, portNumber) < 0){
            if (debug) printf("MasterSwitch unsuccessful\n");
        }else{
            if (debug) printf("MasterSwitch successful\n");
        }
    }

    if (argc == 8){
        if (argv[1][0] == 'p' && argv[1][1] == 's' && argv[1][2] == 'w'){
            int pswi = 0, pswj = 0, pswk = 0, IPlow = 0, IPhigh = 0, portNumber = 9293;
            char psi[2]; psi[0] = argv[1][3]; psi[1] = '\0';
            sscanf(psi, "%d", &pswi);
            psi[0] = argv[3][3];
            if (argv[3][0] == 'n') pswj = -1;
            else sscanf(psi, "%d", &pswj);
            psi[0] = argv[4][3];
            if (argv[4][0] == 'n') pswk = -1;
            else sscanf(psi, "%d", &pswk);
            char iplow[50], iphigh[50], temp[50];
            memset(iplow, 0, 50); memset(iphigh, 0, 50); memset(temp, 0, 50);
            int val = 0, checker = 0;
            int i = 0;
            // TODO: Need to fix IPhigh
            for (i = 0; i < sizeof(argv[5]); i++){
                if (argv[5][i] == '-'){
                    temp[i] = '\0';
                    sscanf(temp, "%d", &IPlow);
                    memset(temp, 0, 50);
                    checker = 1;
                    val = i + 1;
                }
                else{
                    temp[i-val] = argv[5][i];
                }
            }
            temp[i+1] = '\0';
            sscanf(temp, "%d", &IPhigh);
            sscanf(argv[7], "%d", &portNumber);
            if (debug) printf("pswi= %d, pswj= %d, pswk= %d, IPlow= %d, IPhigh= %d, protNumber= %d\n", pswi, pswj, pswk, IPlow, IPhigh, portNumber);
            if (PacketSwitch(pswi, argv[2], pswj, pswk, IPlow, IPhigh, argv[6], portNumber) < 0){
                if (debug) printf("PacketSwitch unsuccessful\n");}
            else{
                if (debug) printf("PacketSwitch successful\n");
            }
        }
        else{
            WrongInput(); return 0;
        }
    }


    return 0;
}