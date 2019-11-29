/*************************************************************************
#	 FileName	: server.c
#	 Author		: fengjunhui 
#	 Email		: 18883765905@163.com 
#	 Created	: 2018年12月29日 星期六 13时44分59秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

#include "common.h"

sqlite3 *db;  //仅服务器使用

int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{	
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;

	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	//printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		//printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			printf("--------------%s登录-----------\n",msg->username);
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	return 0;	
}
//普通用户修改
int process_user_modify_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char sql1[DATALEN] = {0};
	char *errmsg;
	char **result;
	int i=0;
	int nrow,ncolumn;
	time_t t;
	t=time(NULL);
	//根据recmsg的信息来判断客户端选择的式修改地址  电话还是密码
	if(msg->flags == 1){
		sprintf(sql,"update usrinfo set addr='%s' where name = '%s'",msg->info.addr,msg->username);
		//判断式否修改成功  失败打印错误信息 成功则send并插入historyinfo
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{

			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','修改工号为%d的地址为%s')",ctime(&t),msg->username,msg->info.no,msg->info.addr);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 2){
		sprintf(sql,"update usrinfo set phone='%s' where name = '%s'",msg->info.phone,msg->username);
		//printf("%s----\n",sql);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);	
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','修改工号为%d的电话为%s')",ctime(&t),msg->username,msg->info.no,msg->info.phone);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 3){
		sprintf(sql,"update usrinfo set passwd='%s' where name = '%s'",msg->info.passwd,msg->username);
		//printf("%s----\n",sql);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','修改工号为%d的密码为%s')",ctime(&t),msg->username,msg->info.no,msg->info.passwd);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}
}
//普通用户查询
int process_user_query_request(int acceptfd,MSG *msg)
{
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int i=0;
	int nrow,ncolumn;
	char buf[DATALEN] = {};
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	//因为用户只能查询自己的信息所有直接从数据库中提取信息
	sprintf(sql,"select * from usrinfo where usertype=%d and name = '%s' ",msg->usertype,msg->username);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		for(i = 0 ;i < ncolumn ; i++){
			strcat(buf,*(ncolumn+result+i));
			strcat(buf,"|");
		}
		strcpy(msg->recvmsg,buf);
		send(acceptfd , msg , sizeof(MSG),0);
	}
		printf("-----------%s查询了数据----------\n",msg->username);
}
//管理员修改
int process_admin_modify_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char sql1[DATALEN] = {0};
	char sql2[DATALEN] = {0};
	char *errmsg;
	char **result;
	int i=0;
	int nrow,ncolumn;
	time_t t;
	t=time(NULL);
	//优化？
	//根据flags判断用户的选项
	sprintf(sql2,"select * from usrinfo where staffno=%d",msg->info.no);
	if(sqlite3_get_table(db,sql2,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("****%s******\n",errmsg);
	}else if(nrow == 0){
		strcpy(msg->recvmsg,"NULL");
		send(acceptfd , msg , sizeof(MSG) , 0);
	}else if(msg->flags == 1){
		sprintf(sql,"update usrinfo set name='%s' where staffno = %d",msg->info.name,msg->info.no);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的用户名为%s')",ctime(&t),msg->username,msg->info.no,msg->info.name);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 2){
		sprintf(sql,"update usrinfo set age=%d where staffno = %d",msg->info.age,msg->info.no);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的年龄为%d')",ctime(&t),msg->username,msg->info.no,msg->info.age);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 3){
		sprintf(sql,"update usrinfo set addr='%s' where staffno = %d",msg->info.addr,msg->info.no);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的地址为%s')",ctime(&t),msg->username,msg->info.no,msg->info.addr);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 4){
		sprintf(sql,"update usrinfo set phone='%s' where staffno = %d",msg->info.phone,msg->info.no);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的电话为%s')",ctime(&t),msg->username,msg->info.no,msg->info.phone);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 5){
		sprintf(sql,"update usrinfo set work='%s' where staffno = %d",msg->info.work,msg->info.no);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的职位为%s')",ctime(&t),msg->username,msg->info.no,msg->info.work);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 6){
		sprintf(sql,"update usrinfo set salary=%.2f where staffno = %d",msg->info.salary,msg->info.no);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的工资为%f')",ctime(&t),msg->username,msg->info.no,msg->info.salary);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 7){
		sprintf(sql,"update usrinfo set date='%s' where staffno = %d",msg->info.date,msg->info.no);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的入职日期为%s')",ctime(&t),msg->username,msg->info.no,msg->info.date);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 8){
		sprintf(sql,"update usrinfo set level=%d where staffno = %d",msg->info.level,msg->info.no);
		//printf("%s----\n",sql);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的评级为%d')",ctime(&t),msg->username,msg->info.no,msg->info.level);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}else if(msg->flags == 9){
		sprintf(sql,"update usrinfo set passwd='%s' where staffno = %d",msg->info.passwd,msg->info.no);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s\n",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员修改工号为%d的密码为%s')",ctime(&t),msg->username,msg->info.no,msg->info.passwd);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s修改了数据----------\n",msg->username);
		}
	}
}
//管理员添加用户
int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char sql1[DATALEN] = {0};
	char sql2[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	time_t t; 
	t = time(NULL);
	sprintf(sql2,"select * from usrinfo where staffno=%d",msg->info.no);
	if(sqlite3_get_table(db,sql2,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("*****%s****\n",errmsg);
		strcpy(msg->recvmsg,"ERR");
		send(acceptfd , msg , sizeof(MSG) , 0);
	}else if(nrow != 0){
		strcpy(msg->recvmsg,"EXIT");
		send(acceptfd , msg , sizeof(MSG) , 0);
	}else{
		sprintf(sql,"insert into usrinfo values(%d,%d,'%s','%s',%d,'%s','%s','%s','%s',%d,%.1f)",msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员添加工号为%d的员工')",ctime(&t),msg->username,msg->info.no);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s添加了了用户%s----------\n",msg->username,msg->info.name);
		}
	}
}
//管理员删除
int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char sql1[DATALEN] = {0};
	char sql2[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	time_t t;
	t = time(NULL);
	sprintf(sql2,"select * from usrinfo where staffno=%d",msg->info.no);
	if(sqlite3_get_table(db,sql2,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("*****%s****\n",errmsg);
		strcpy(msg->recvmsg,"ERR");
		send(acceptfd , msg , sizeof(MSG) , 0);
	}else if(nrow == 0){
		strcpy(msg->recvmsg,"NULL");
		send(acceptfd , msg , sizeof(MSG) , 0);
	}else{
		sprintf(sql,"delete from usrinfo where staffno=%d and name='%s'",msg->info.no,msg->info.name);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			printf("%s",errmsg);
			strcpy(msg->recvmsg,"ERR");
			send(acceptfd , msg , sizeof(MSG) , 0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd , msg , sizeof(MSG) , 0);
			sprintf(sql1,"insert into historyinfo values('%s','%s','管理员删除工号为%d的员工')",ctime(&t),msg->username,msg->info.no);
			if(sqlite3_exec(db,sql1,NULL,NULL,&errmsg) != SQLITE_OK){
				printf("%s\n",errmsg);
			}
			printf("-----------%s删除了用户%s----------\n",msg->username,msg->info.name);
		}
	}
}
//管理员查询
int process_admin_query_request(int acceptfd,MSG *msg)
{
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int i=0,j=0;
	int nrow,ncolumn;
	char buf[DATALEN] = {};
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	if(strncmp(msg->recvmsg,"1",1) == 0){
		sprintf(sql,"select * from usrinfo where name = '%s'",msg->info.name);
		printf("-----------%s查询了用户%s----------\n",msg->username,msg->info.name);
		if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
			printf("---****----%s.\n",errmsg);	
		//判断有没有这个用户
		}else if(nrow != 0){
			for(i = 0 ;i < ncolumn ; i++){
				strcat(buf,*(ncolumn+result+i));
				strcat(buf,"|");
			}
			strcpy(msg->recvmsg,buf);
			send(acceptfd , msg , sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"NULL");
			send(acceptfd , msg , sizeof(MSG),0);
			
		}
	}
	if(strncmp(msg->recvmsg,"2",1) == 0){
		printf("-----------%s查询了所有用户----------\n",msg->username);
		sprintf(sql,"select * from usrinfo");
		if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
			printf("---****----%s.\n",errmsg);		
		}else{
			for(j = 1; j < nrow+1 ; j++){
				for(i = 0 ;i < ncolumn ; i++){
					strcat(buf,*(j*ncolumn+result+i));
					strcat(buf,"|");
				}
				strcpy(msg->recvmsg,buf);
				send(acceptfd , msg , sizeof(MSG),0);
				bzero(buf,sizeof(buf));
			}
			strcpy(msg->recvmsg,"ALL");
			send(acceptfd , msg ,sizeof(MSG) , 0);
		}
	}
}
//管理员查询历史记录
int process_admin_history_request(int acceptfd,MSG *msg)
{
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int i=0,j=0;
	int nrow,ncolumn;
	char buf[DATALEN] = {};
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	sprintf(sql,"select * from historyinfo");
	printf("-----------%s查询了历史记录----------\n",msg->username);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		for(j = 1; j < nrow+1 ; j++){
			for(i = 0 ;i < ncolumn ; i++){
				strcat(buf,*(j*ncolumn+result+i));
				strcat(buf,"----");
			}
			strcpy(msg->recvmsg,buf);
			send(acceptfd , msg , sizeof(MSG),0);
			bzero(buf,sizeof(buf));
		}
		strcpy(msg->recvmsg,"over");
		send(acceptfd , msg ,sizeof(MSG) , 0);
	}
}
//?
int process_client_quit_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	printf("用户退出\n");
}
//主页
int process_client_request(int acceptfd,MSG *msg)
{
//	printf("------------%s-----------%d.\n",__func__,__LINE__);
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(acceptfd,msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(acceptfd,msg);
			break;
		case USER_QUERY:
			process_user_query_request(acceptfd,msg);
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(acceptfd,msg);
			break;
		case ADMIN_ADDUSER:
			process_admin_adduser_request(acceptfd,msg);
			break;
		case ADMIN_DELUSER:
			process_admin_deluser_request(acceptfd,msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(acceptfd,msg);
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(acceptfd,msg);
			break;
		case QUIT:
			process_client_quit_request(acceptfd,msg);
			break;
		default:
			break;
	}
}
int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	MSG msg;
	//thread_data_t tid_data;
	char *errmsg;

	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table usrinfo(staffno integer,usertype integer,name text,passwd text,age integer,phone text,addr text,work text,date text,level integer,salary REAL);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create usrinfo table success.\n");
	}

	if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{ //华清远见创客学院         嵌入式物联网方向讲师
		printf("create historyinfo table success.\n");
	}

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 

	
	/*优化4： 允许绑定地址快速重用 */
	int b_reuse = 1;
	setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));
	
	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
//	serveraddr.sin_port   = htons(5001);
//	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.200");


	//绑定网络套接字和网络结构体
	if(bind(sockfd, (const struct sockaddr *)&serveraddr,addrlen) == -1){
		printf("bind failed.\n");
		exit(-1);
	}

	//监听套接字，将主动套接字转化为被动套接字
	if(listen(sockfd,10) == -1){
		printf("listen failed.\n");
		exit(-1);
	}

	//定义一张表
	fd_set readfds,tempfds;
	//清空表
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	//添加要监听的事件
	FD_SET(sockfd,&readfds);
	int nfds = sockfd;
	int retval;
	int i = 0;

#if 0 //添加线程控制部分
	pthread_t thread[N];
	int tid = 0;
#endif

	while(1){
		tempfds = readfds;
		//记得重新添加
		retval =select(nfds + 1, &tempfds, NULL,NULL,NULL);
		//判断是否是集合里关注的事件
		for(i = 0;i < nfds + 1; i ++){
			if(FD_ISSET(i,&tempfds)){
				if(i == sockfd){
					//数据交互 
					acceptfd = accept(sockfd,(struct sockaddr *)&clientaddr,&cli_len);
					if(acceptfd == -1){
						printf("acceptfd failed.\n");
						exit(-1);
					}
					printf("ip : %s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&readfds);
					nfds = nfds > acceptfd ? nfds : acceptfd;
				}else{
					recvbytes = recv(i,&msg,sizeof(msg),0);
					printf("msg.type :%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recv failed.\n");
						continue;
					}else if(recvbytes == 0){
						printf("peer shutdown.\n");
						close(i);
						FD_CLR(i, &readfds);  //删除集合中的i
					}else{
						process_client_request(i,&msg);
					}
				}
			}
		}
	}
	close(sockfd);

	return 0;
}







#if 0
					//tid_data.acceptfd = acceptfd;   //暂时不使用这种方式
					//tid_data.state	  = 1;
					//tid_data.thread   = thread[tid++];	
					//pthread_create(&tid_data.thread, NULL,client_request_handler,(void *)&tid_data);
#endif 

#if 0
void *client_request_handler(void * args)
{
	thread_data_t *tiddata= (thread_data_t *)args;

	MSG msg;
	int recvbytes;
	printf("tiddata->acceptfd :%d.\n",tiddata->acceptfd);

	while(1){  //可以写到线程里--晚上的作业---- UDP聊天室
		//recv 
		memset(msg,sizeof(msg),0);
		recvbytes = recv(tiddata->acceptfd,&msg,sizeof(msg),0);
		if(recvbytes == -1){
			printf("recv failed.\n");
			close(tiddata->acceptfd);
			pthread_exit(0);
		}else if(recvbytes == 0){
			printf("peer shutdown.\n");
			pthread_exit(0);
		}else{
			printf("msg.recvmsg :%s.\n",msg.recvmsg);
			strcat(buf,"*-*");
			send(tiddata->acceptfd,&msg,sizeof(msg),0);
		}
	}

}

#endif 













