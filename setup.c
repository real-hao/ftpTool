/*
 * @Author: Hao
 * @Date: 2021-02-25 19:08:23
 * @LastEditors: Hao
 * @LastEditTime: 2021-02-26 12:40:46
 * @Description: 
 * @FilePath: /ftptool/setup.c
 */

#include "ftptool.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

/***************************************************************
 *
 *                  ,----------------,              ,---------,
 *             ,-----------------------,          ,"        ,"|
 *           ,"       ftpTool        ,"|        ,"        ,"  |
 *          +-----------------------+  |      ,"        ,"    |
 *          |  .-----------------.  |  |     +---------+      |
 *          |  |                 |  |  |     | -==----'|      |
 *          |  |  >ftp tool      |  |  |     |         |      |
 *          |  |  >get MyLove.sh |  |  |/----|`---=    |      |
 *          |  |  >ls            |  |  |   ,/|==== ooo |      ;
 *          |  |                 |  |  |  // |(((( [33]|    ,"
 *          |  `-----------------'  |," .;'| |((((     |  ,"
 *          +-----------------------+  ;;  | |         |,"
 *            /_)______________(_/   //'   | +---------+
 *       ___________________________/___   `,
 *      /  oooooooooooooooo  .o.  oooo /,   \,"-----------
 *     / ==ooooooooooooooo==.o.  ooo= //   ,`\--{)B     ,"
 *    /_==__==========__==_ooo__ooo=_/'   /___________,"
 *
 ****************************************************************/

typedef enum tips{
    PARAM_NUM_ERROR,  TIPS_CLOULE, TIPS_LOCAL, 
}Tips;

static int obj = OBJ_CLOULE;

void printInfo(char* ip, int port, int maxCon);
void printTips(Tips code);

int main(int argc, char* argv[]){
    if(argc < 2 || argc > 4){
        printTips(PARAM_NUM_ERROR);
        return 0;
    }
    if(!strcmp(argv[1], "--client")){
        signal(SIGINT, SIG_IGN);
        int port = 8080;
        switch (argc)
        {
        case 4:
            if((port = atoi(argv[3])) == 0){
                port = 8080;
                printTips(PARAM_NUM_ERROR);
            }
        case 3:
            printInfo(argv[2], port, 0);
            char buf[MAXSIZE_COMMAND];
            do{
                (obj == OBJ_CLOULE) ? printTips(TIPS_CLOULE) : printTips(TIPS_LOCAL);
                memset(buf, '\0', MAXSIZE_COMMAND);
                fgets(buf, MAXSIZE_COMMAND, stdin);
                buf[strlen(buf) - 1] = '\0';
                if(!strcmp(buf, "q")){
                    break;
                }else if(!strcmp(buf, "c")){
                    obj = -obj;
                }else{
                    TransactionResult result =  execTransaction(argv[2], port, buf, obj);
                    if(result != TRANSACTION_DONE_OK){
                        printTransactionResult(result);
                    }
                }
            }while(1);
            break;
        case 2:
            printTips(PARAM_NUM_ERROR);
        }
    }else if(!strcmp(argv[1], "--server")){
        int port = 8080, maxCon = 10;
        switch (argc)
        {
        case 4:
            if((port = atoi(argv[3])) == 0){
                port = 8080;
                printTips(PARAM_NUM_ERROR);
            }
        case 3:
            if((maxCon = atoi(argv[2])) == 0){
                maxCon = 10;
                printTips(PARAM_NUM_ERROR);
            }
        }
        printInfo(NULL, port, maxCon);
        initServer(port, maxCon);
    }else{
        printTips(PARAM_NUM_ERROR);
    }
    return 0;
}

void printInfo(char* ip, int port, int maxCon){
    if(ip != NULL){
        printf("ip: %s.", ip);
    }
    if(port != 0){
        printf("port: %d.", port);
    }
    if(maxCon > 0){
        printf("maxCon: %d", maxCon);
    }
    putchar('\n');
}

void printTips(Tips code){
    switch(code){
        case PARAM_NUM_ERROR:
            puts("param num error, try man 'ftptool' for help");
        break;
        case TIPS_CLOULE:
            printf("cloud> ");
        break;
        case TIPS_LOCAL:
            printf("local> ");
        break;
    }
}
