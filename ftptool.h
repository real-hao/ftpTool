/*
 * @Author: Hao
 * @Date: 2021-02-25 19:09:04
 * @LastEditors: Hao
 * @LastEditTime: 2021-02-26 12:29:48
 * @Description: 
 * @FilePath: /ftptool/ftptool.h
 */

/******************************************
 *
 *  ██████╗ ██╗   ██╗ ██████╗
 *  ██╔══██╗██║   ██║██╔════╝
 *  ██████╔╝██║   ██║██║  ███╗
 *  ██╔══██╗██║   ██║██║   ██║
 *  ██████╔╝╚██████╔╝╚██████╔╝
 *  ╚═════╝  ╚═════╝  ╚═════╝
 * 
 ******************************************/

#ifndef _FTP_TOOL_H
#define _FTP_TOOL_H

#define MAXSIZE_COMMAND 255+5

#define OBJ_LOCAL 1
#define OBJ_CLOULE -1

typedef enum transactionResult{
    SOCKET_INIT_EXCEPTION, SOCKET_CONNECT_EXCEPTION, INPUT_FORMAT_EXCEPTION, TRANSACTION_DONE_OK, 
}TransactionResult;

extern TransactionResult execTransaction(char* ip, int port, char* input, int obj);
extern void printTransactionResult(TransactionResult resultCode);

extern void initServer(int port, int maxCon);

#endif
