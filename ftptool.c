/*
 * @Author: Hao
 * @Date: 2021-02-25 16:31:47
 * @LastEditors: Hao
 * @LastEditTime: 2021-02-26 18:43:12
 * @Description: 
 * @FilePath: /ftptool/ftptool.c
 */

//***********************************************************************
// 
//                 .-~~~~~~~~~-._       _.-~~~~~~~~~-.
//             __.'              ~.   .~              `.__
//           .'//   hello          \./     world        \\`.
//         .'//     ftp             |      tool           \\`.
//       .'// .-~"""""""~~~~-._     |     _,-~~~~"""""""~-. \\`.
//     .'//.-"                 `-.  |  .-'                 "-.\\`.
//   .'//______.============-..   \ | /   ..-============.______\\`.
// .'______________________________\|/______________________________`.
// 
//***********************************************************************

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include "ftptool.h"

#define MAXSIZE_BUFFER 4096
#define MAXSIZE_LONG 8+1

static const char* FLAG_CHDIR = "cd ";
static const char* FLAG_GET = "get ";
static const char* FLAG_PUT = "put ";
static const char* FLAG_RM = "rm ";

static int counter = 0;
static int server_fd = 0;

typedef enum commandType{
    CMD_CHDIR, CMD_GET, CMD_PUT, CMD_ERROR, CMD_RM, 
}CommandType;

typedef enum socketStatus{
    SOCKET_OK, SOCKET_SEND_ERROR, SOCKET_RECEIVE_ERROR, 
    SOCKET_FILE_NOT_EXIST, SOCKET_FILE_READ_ERROR, SOCKET_FILE_CREATE_ERROR, 
}SocketStatus;

typedef enum tips{
    TIPS_SIGNAL_ERROR, TIPS_STOP_ERROR, TIPS_STOP_OK, 
    TIPS_INIT_ERROR, TIPS_BIND_ERROR, TIPS_LISTEN_ERROR, TIPS_ACCEPT_ERROR, 
    TIPS_THREAD_ERROR
}Tips;

static CommandType parseCommand(char* command, char* param);
static SocketStatus sendCommand(int fd, char* source);
static SocketStatus receiveCommand(int fd, char* source);
static SocketStatus sendData(int fd, char* path, void(*callback)(long readSize, long totalSize, char* fileName));
static SocketStatus receiveData(int fd, char* fileName, void(*callback)(long readSize, long totalSize, char* fileName));
static void printSocketStatus(SocketStatus statusCode);
static void callback(long readSize, long totalSize, char* fileName);
static void printTips(Tips code);

extern TransactionResult execTransaction(char* ip, int port, char* input, int obj){
    char param[MAXSIZE_COMMAND];
    CommandType cmdTyp = parseCommand(input, param);
    if(cmdTyp == CMD_ERROR){
        return INPUT_FORMAT_EXCEPTION;
    }
    if(obj == OBJ_LOCAL && (cmdTyp == CMD_CHDIR || cmdTyp == CMD_RM)){
        if(cmdTyp == CMD_CHDIR){
            chdir(param);
        }else{
            system(input);
        }
        system("pwd");
        puts("----------------------------------------");
        system("ls -a");
        return TRANSACTION_DONE_OK;
    }
    int clnt_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(clnt_fd < 0){
        return SOCKET_INIT_EXCEPTION;
    }
    struct sockaddr_in c_addr;
    memset(&c_addr, 0, sizeof(struct sockaddr_in));
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(port);
    inet_aton(ip, &c_addr.sin_addr);
    int rc = connect(clnt_fd, (struct sockaddr*)&c_addr, sizeof(struct sockaddr_in));
    if (rc < 0){
        return SOCKET_CONNECT_EXCEPTION;
    }
    SocketStatus sktStas = sendCommand(clnt_fd, input);
    if(sktStas != SOCKET_OK){
        printSocketStatus(sktStas);
        return -1;
    }
    switch(cmdTyp){
        case CMD_GET:
            sktStas = receiveData(clnt_fd, param, callback);
            break;
        case CMD_PUT:
            sktStas = sendData(clnt_fd, param, callback);
            break;
        case CMD_RM:
        case CMD_CHDIR:
            sktStas = receiveData(clnt_fd, NULL, NULL);
            break;
    }
    if(sktStas != SOCKET_OK){
        printSocketStatus(sktStas);
    }
    shutdown(clnt_fd, SHUT_RDWR);
    close(clnt_fd);
    return TRANSACTION_DONE_OK;
}

static void handler(int signal){
    putchar('\n');
    if(signal == SIGINT){
        if(counter){
            printTips(TIPS_STOP_ERROR);
        }else{
            close(server_fd);
            printTips(TIPS_STOP_OK);
            exit(0);
        }
    }
}

static void* transactionThread(void* arg){
    int fd = *(int*)arg;
    int rc;
    char command[MAXSIZE_COMMAND];
    char param[MAXSIZE_COMMAND];
    rc = receiveCommand(fd, command);
    if(rc != SOCKET_OK){
        printSocketStatus(rc);
        goto end;
    }
    CommandType cmdTyp = parseCommand(command, param);
    SocketStatus sktSta;
    switch (cmdTyp)
    {
    case CMD_GET:
        sktSta = sendData(fd, param, callback);
        break;
    case CMD_PUT:
        sktSta = receiveData(fd, param, callback);
        break;
    case CMD_CHDIR:
    case CMD_RM:
        if(cmdTyp == CMD_CHDIR){
            chdir(param);
        }else{
            system(command);
        }
        system("pwd > .tmp");
        system("echo ---------------------------------------- >> .tmp");
        system("ls -a >> .tmp");
        sktSta = sendData(fd, ".tmp", callback);
        system("rm .tmp");
        break;
    default:
        break;
    }
    if(sktSta != SOCKET_OK){
        printSocketStatus(sktSta);
    }
    end:
	counter --;
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

extern void initServer(int port, int maxCon){
    if(signal(SIGINT, handler) == SIG_ERR){
        printTips(TIPS_SIGNAL_ERROR);
        return;
    }
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        printTips(TIPS_INIT_ERROR);
        return;
    }
    struct sockaddr_in s_addr;
    memset(&s_addr, 0, sizeof(struct sockaddr_in));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    inet_aton("", &s_addr.sin_addr);
    int rc;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    rc = bind(server_fd, (struct sockaddr*)&s_addr, sizeof(struct sockaddr_in));
    if(rc < 0){
        printTips(TIPS_BIND_ERROR);
        return;
    }
    rc = listen (server_fd, maxCon);
    if(rc < 0){
        printTips(TIPS_LISTEN_ERROR);
        return;
    }
    struct sockaddr_in c_addr;
    memset(&c_addr, 0, sizeof(struct sockaddr_in));
    int size = sizeof(struct sockaddr_in);
    while(1){
        int fd = accept(server_fd, (struct sockaddr*)&c_addr, &size);
        if(fd < 0){
            printTips(TIPS_ACCEPT_ERROR);
            break;
        }
        counter ++;
        puts(inet_ntoa(c_addr.sin_addr));
        pthread_t t;
        rc = pthread_create(&t, NULL, transactionThread, (void*)&fd);
        if(rc != 0){
            printTips(TIPS_THREAD_ERROR);
            continue;
        }
    }
}

static CommandType parseCommand(char* command, char* param){
    CommandType ret = CMD_ERROR;
    if(!strncmp(command, FLAG_CHDIR, strlen(FLAG_CHDIR))){
        ret = CMD_CHDIR;
    }else if(!strncmp(command, FLAG_GET, strlen(FLAG_GET))){
        ret = CMD_GET;
    }else if(!strncmp(command, FLAG_PUT, strlen(FLAG_PUT))){
        ret = CMD_PUT;
    }else if(!strncmp(command, FLAG_RM, strlen(FLAG_RM))){
        ret = CMD_RM;
    }
    if(param != NULL && ret != CMD_ERROR){
        strcpy(param, strstr(command, " ") + 1);
        if(strlen(param) == 0){
            ret = CMD_ERROR;
        }
    }
    return ret;
}

static void callback(long readSize, long totalSize, char* fileName){
    long process = (readSize*1.0 / totalSize) * 100;
    int barSize = (process+2) / 2;
    char bar[barSize];
    memset(bar, '#', barSize);
    bar[barSize - 1] = '\0';
    printf("\r%s [%ldKB/%ldKB] %ld%% %s", fileName, readSize, totalSize, process, bar);
    fflush(stdout);
    if(process >= 100){
        putchar('\n');
    }
}

static SocketStatus sendCommand(int fd, char* source){
    int rc = write(fd, source, MAXSIZE_COMMAND);
    if(rc < 0){
        return SOCKET_SEND_ERROR;
    }
    return SOCKET_OK;
}

static SocketStatus receiveCommand(int fd, char* source){
    int rc = read(fd, source, MAXSIZE_COMMAND);
    if(rc < 0){
        return SOCKET_RECEIVE_ERROR;
    }
    return SOCKET_OK;
}

static SocketStatus sendData(int fd, char* path, void(*callback)(long readSize, long totalSize, char* fileName)){
    FILE* fp = fopen(path, "rb");
    char buf[MAXSIZE_BUFFER];
    memset(buf, '\0', MAXSIZE_BUFFER);
    long readSize, totalSize;
    int rc;
    if(fp == NULL){
        return SOCKET_FILE_NOT_EXIST;
    }
    fseek(fp, 0L, SEEK_END);
    totalSize = ftell(fp);
    rewind(fp);
    if(totalSize < 0){
        fclose(fp);
        return SOCKET_FILE_READ_ERROR;
    }
    sprintf(buf, "%ld", totalSize);
    rc = write(fd, buf, MAXSIZE_LONG);
    if(rc < 0){
        fclose(fp);
        return SOCKET_SEND_ERROR;
    }
    readSize = 0;
    do{
        rc = fread(buf, 1, MAXSIZE_BUFFER, fp);
        if(rc <= 0){
            break;
        }
        if(write(fd, buf, rc) != rc){
            break;
        }
        readSize += rc;
        callback(readSize, totalSize, path);
    }while(readSize < totalSize);
    fclose(fp);
    if(readSize != totalSize){
        return SOCKET_SEND_ERROR;
    }
    return SOCKET_OK;
}

static SocketStatus receiveData(int fd, char* fileName, void(*callback)(long readSize, long totalSize, char* fileName)){
    char buf[MAXSIZE_BUFFER];
    memset(buf, '\0', MAXSIZE_BUFFER);
    int rc;
    long readSize, totalSize;
    rc = read(fd, buf, MAXSIZE_LONG);
    if(rc < 0){
        return SOCKET_RECEIVE_ERROR;
    }
    totalSize = atoi(buf);
    if(totalSize < 0){
        return SOCKET_RECEIVE_ERROR;
    }
    FILE* fp;
    if(fileName != NULL){
        fp = fopen(fileName, "wb+");
        if(fp == NULL){
            return SOCKET_FILE_CREATE_ERROR;
        }
    }else{
        fp = stdout;
    }
    readSize = 0;
    if(totalSize != 0){
        do{
            rc = read(fd, buf, MAXSIZE_BUFFER);
            if(rc < 0){
                break;
            }
            if(fwrite(buf, 1, rc, fp) != rc){
                break;
            }
            readSize += rc;
            if(fp != stdout) callback(readSize, totalSize, fileName);
        }while(readSize < totalSize);
    }
    if(fp != stdout){
        fclose(fp);
    }
    if(readSize != totalSize){
        return SOCKET_RECEIVE_ERROR;
    }
    return SOCKET_OK;
}

extern void printTransactionResult(TransactionResult resultCode){
    switch(resultCode){
    case SOCKET_INIT_EXCEPTION:
        puts("socket init exception");
        break;
    case SOCKET_CONNECT_EXCEPTION:
        puts("socket connect exception");
        break;
    case INPUT_FORMAT_EXCEPTION:
        puts("input format exception");
        break;
    case TRANSACTION_DONE_OK:
        puts("transaction done ok");
        break;
    }
}

static void printSocketStatus(SocketStatus statusCode){
    switch(statusCode){
        case SOCKET_OK:
            puts("socket ok");
            break;
        case SOCKET_SEND_ERROR:
            puts("socket send error");
            break;
        case SOCKET_RECEIVE_ERROR:
            puts("socket receive error");
            break;
        case SOCKET_FILE_NOT_EXIST:
            puts("socket file not exist");
            break;
        case SOCKET_FILE_READ_ERROR:
            puts("socket file read error");
            break;
        case SOCKET_FILE_CREATE_ERROR:
            puts("socket file create error");
            break;
    }
}

static void printTips(Tips code){
    switch(code){
        case TIPS_SIGNAL_ERROR:
            puts("tips signal error");
            break;
        case TIPS_STOP_ERROR:
            puts("tips stop error");
            break;
        case TIPS_STOP_OK:
            puts("tips stop ok");
            break; 
        case TIPS_THREAD_ERROR:
            puts("tips thread error");
            break;
    }
}
