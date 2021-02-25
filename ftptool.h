/*
 * @Author: Hao
 * @Date: 2021-02-25 19:09:04
 * @LastEditors: Hao
 * @LastEditTime: 2021-02-25 23:37:41
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

typedef enum transactionResult{
    SOCKET_INIT_EXCEPTION, SOCKET_CONNECT_EXCEPTION, INPUT_FORMAT_EXCEPTION, TRANSACTION_DONE_OK, 
}TransactionResult;

extern TransactionResult execTransaction(char* ip, int port, char* input);
extern void printTransactionResult(TransactionResult resultCode);

extern void initServer(int port, int maxCon);

#endif
