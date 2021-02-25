/*
 * @Author: Hao
 * @Date: 2021-02-25 19:08:23
 * @LastEditors: Hao
 * @LastEditTime: 2021-02-25 23:37:11
 * @Description: 
 * @FilePath: /ftptool/setup.c
 */

#include "ftptool.h"
#include <stdio.h>
#include <string.h>

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

int main(int argc, char* argv[]){
    if(!strcmp(argv[1], "--client")){
        TransactionResult result =  execTransaction("192.168.8.105", 8080, "cd ..");
        if(result != TRANSACTION_DONE_OK){
            printTransactionResult(result);
        }
    }else if(!strcmp(argv[1], "--server")){
        initServer(8080, 10);
    }
    return 0;
}
