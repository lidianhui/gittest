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

#include "common.h"
void admin_menu(int sockfd,MSG *msg);

void user_menu(int sockfd,MSG *msg);
/**************************************
 *功   能：管理员查询
 ****************************************/
void do_admin_query(int sockfd,MSG *msg)
{
	int num;
	char name[DATALEN];
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_QUERY;
	printf("********************************************************\n");
	printf("******* 1：按人名查找  	2：查找所有 	3：退出	 *******\n");
	printf("********************************************************\n");
	printf("请输入您的选择（数字）>> ");
	scanf("%d",&num);
	getchar();
	switch(num){
		case 1:
			strcpy(msg->recvmsg,"1");
			printf("请输入要查找的用户名：");
			scanf("%s",name);
			getchar();
			strcpy(msg->info.name , name);
			send(sockfd , msg , sizeof(MSG) , 0);
			recv(sockfd , msg , sizeof(MSG) , 0);
			if( (strncmp(msg->recvmsg,"NULL",4)) == 0){
				printf("查无此人\n");
			}else{
				printf("工号|用户类型|姓名|密码|年龄|电话|地址|职位|入职年月|等级|工资\n");
				printf("===============================================================\n");
				printf("%s\n",msg->recvmsg);
			}
			break;
		case 2:
			strcpy(msg->recvmsg,"2");
			send(sockfd , msg , sizeof(MSG) , 0);
			printf("工号|用户类型|姓名|密码|年龄|电话|地址|职位|入职年月|等级|工资\n");
			printf("===============================================================\n");
			while(1){
				recv(sockfd , msg , sizeof(MSG) , 0);
				if(strncmp(msg->recvmsg,"ALL",3) == 0){
					break;
				}
				printf("%s\n",msg->recvmsg);
			}
			break;
		case 3:
			admin_menu(sockfd,msg);
			break;
		default:
			printf("输入错误\n");
	}
}

/**************************************
 *功   能：管理员修改
 ****************************************/
void do_admin_modification(int sockfd,MSG *msg)
{
	int no,num;
	//union?
	int n_num;
	char n_buf[DATALEN];
	float n_f;
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_MODIFY;
	printf("请输入您要修改的人的工号：");
	scanf("%d",&no);
	getchar(); 
	msg->info.no = no;
	printf("*******************请输入要修改的选项********************\n");
	printf("******	1：姓名	  2：年龄	3：家庭住址   4：电话  ******\n");
	printf("******	5：职位	  6：工资   7：入职年月   8：评级  ******\n");
	printf("******	9：密码	  10：退出				           ******\n");
	printf("*********************************************************\n");
	printf("请输入你的选择>>");
	scanf("%d",&num);
	getchar();
	switch(num){
		case 1:
			printf("new name");
			scanf("%s",n_buf);
			getchar();
			strcpy(msg->info.name,n_buf);
			msg->flags = 1;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 2:
			printf("new age");
			scanf("%d",&n_num);
			getchar();
			msg->info.age = n_num;
			msg->flags = 2;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 3:
			printf("new addr");
			scanf("%s",n_buf);
			getchar();
			strcpy(msg->info.addr,n_buf);
			msg->flags = 3;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 4:
			printf("new phone");
			scanf("%s",n_buf);
			getchar();
			strcpy(msg->info.phone,n_buf);
			msg->flags = 4;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 5:
			printf("new work");
			scanf("%s",n_buf);
			getchar();
			strcpy(msg->info.work,n_buf);
			msg->flags = 5;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 6:
			printf("new salary");
			scanf("%f",&n_f);
			getchar();
			msg->info.salary = n_f;
			msg->flags = 6;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 7:
			printf("new date");
			scanf("%s",n_buf);
			getchar();
			strcpy(msg->info.date,n_buf);
			msg->flags = 7;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 8:
			printf("new level");
			scanf("%d",&n_num);
			getchar();
			msg->info.level = n_num;
			msg->flags = 8;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 9:
			printf("new passwd");
			scanf("%s",n_buf);
			getchar();
			strcpy(msg->info.passwd,n_buf);
			msg->flags = 9;
			send(sockfd , msg , sizeof(MSG) ,0);
			break;
		case 10:
			admin_menu(sockfd,msg);
			break;
		default:
			printf("输入错误\n");
			do_admin_modification(sockfd,msg);
	}
	recv(sockfd ,msg ,sizeof(MSG) , 0);
	if((strncmp(msg->recvmsg,"ERR",3)) == 0){
		printf("修改失败\n");
	}else if((strncmp(msg->recvmsg,"OK",2)) == 0){
		printf("数据库修改成功，修改结束\n");
	}else if((strncmp(msg->recvmsg,"NULL",4)) == 0){
		printf("查无此人\n");
	
	}
}
/**************************************
 *功   能：管理员创建用户
 ****************************************/
void do_admin_adduser(int sockfd,MSG *msg)
{		
	char buf[DATALEN];
	int num;
	float fl;
	char n;
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_ADDUSER;
	printf("***************热烈欢迎新员工***************\n");
	printf("请输入工号 ");
	scanf("%d",&num);
	getchar();
	printf("您输入的工号是：%d\n",num);
	printf("工号信息一旦录入无法更改，请确认您所输入的是否正确！\n");
	msg->info.no = num;
	
	printf("请输入用户名：");
	bzero(buf,sizeof(buf));
	scanf("%s",buf);
	getchar();
	strcpy(msg->info.name,buf);
	
	printf("请输入用户密码：");
	bzero(buf,sizeof(buf));
	scanf("%s",buf);
	getchar();
	strcpy(msg->info.passwd,buf);
	
	printf("请输入年龄：");
	scanf("%d",&num);
	getchar();
	msg->info.age = num;
	
	printf("请输入电话：");
	bzero(buf,sizeof(buf));
	scanf("%s",buf);
	getchar();
	strcpy(msg->info.phone,buf);
	
	printf("请输入家庭住址：");
	bzero(buf,sizeof(buf));
	scanf("%s",buf);
	getchar();
	strcpy(msg->info.addr,buf);
	
	printf("请输入职位：");
	bzero(buf,sizeof(buf));
	scanf("%s",buf);
	getchar();
	strcpy(msg->info.work,buf);
	
	printf("请输入入职日期（0000.00.00）：");
	bzero(buf,sizeof(buf));
	scanf("%s",buf);
	getchar();
	strcpy(msg->info.date,buf);
	
	printf("请输入评级（1-5，5为最高，新员工为1）：");
	scanf("%d",&num);
	getchar();
	msg->info.level = num;
	
	printf("请输入工资:");
	scanf("%f",&fl);
	getchar();
	msg->info.salary = fl;
	
	printf("是否为管理员（Y/N）:");
	scanf("%c",&n);
	getchar();
	
	if(n == 'Y' || n == 'y'){
		msg->info.usertype = 0;
	}else{
		msg->info.usertype = 1;
	}
	send(sockfd , msg ,sizeof(MSG) , 0);
	recv(sockfd , msg ,sizeof(MSG) , 0);
	if((strncmp(msg->recvmsg,"OK",2)) == 0){
		printf("数据库修改成功！是否继续添加员工：（Y/N）");
		scanf("%c",&n);
		getchar();
		if(n == 'Y' || n == 'y'){
			do_admin_adduser(sockfd,msg);
		}
	}else if(strncmp(msg->recvmsg,"EXIT",4) == 0){
		printf("职工号已经存在\n");
	}else{
		printf("添加失败\n");
	}
}
/**************************************
 *功   能：管理员删除用户
 ****************************************/
void do_admin_deluser(int sockfd,MSG *msg)
{
	char buf[DATALEN];
	int num;
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_DELUSER;
	printf("请输入要删除的工号：");
	scanf("%d",&num);
	getchar();
	msg->info.no = num;
	printf("请输入要删除的用户名：");
	scanf("%s",buf);
	getchar();
	strcpy(msg->info.name,buf);
	send(sockfd , msg ,sizeof(MSG) , 0);
	recv(sockfd , msg ,sizeof(MSG) , 0);
	if((strncmp(msg->recvmsg,"OK",2)) == 0){
		printf("数据库修改成功！删除用户名为%s的用户",buf);
	}else if(strncmp(msg->recvmsg,"NULL",4) == 0){
		printf("职工号不存在\n");
	}else{
		printf("删除失败\n");
	}
}
/**************************************
 *功   能：查看历史记录
 ****************************************/
void do_admin_history (int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_HISTORY;
	send(sockfd ,msg ,sizeof(MSG) ,0);
	while(1){
		recv(sockfd , msg ,sizeof(MSG) , 0);
		if(strncmp(msg->recvmsg,"over",4) == 0 ){
			printf("%s查询历史记录完成\n",msg->username);
			break;
		}else{
			printf("%s\n",msg->recvmsg);
		}
	}
}

/**************************************
 *功   能：管理员菜单
 ****************************************/
void admin_menu(int sockfd,MSG *msg)
{
	int n;
	while(1)
	{
		printf("*************************************************************\n");
		printf("* 1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录*\n");
		printf("* 6：退出													*\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
				do_admin_query(sockfd,msg);
				break;
			case 2:
				do_admin_modification(sockfd,msg);
				break;
			case 3:
				do_admin_adduser(sockfd,msg);
				break;
			case 4:
				do_admin_deluser(sockfd,msg);
				break;
			case 5:
				do_admin_history(sockfd,msg);
				break;
			case 6:
				msg->msgtype = QUIT;
				send(sockfd, msg, sizeof(MSG), 0);
				close(sockfd);
				exit(0);
			default:
				printf("您输入有误，请重新输入！\n");
		}
	}
}
/**************************************
 *功   能：用户查询
 ****************************************/
void do_user_query(int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = USER_QUERY;
	send(sockfd , msg , sizeof(MSG) , 0);
	recv(sockfd , msg , sizeof(MSG) , 0);
	printf("工号|用户类型|姓名|密码|年龄|电话|地址|职位|入职年月|等级|工资\n");
	printf("===============================================================\n");
	printf("%s\n",msg->recvmsg);
	
}
/**************************************
 *功   能：修改
 ****************************************/
void do_user_modification(int sockfd,MSG *msg)
{	
	int no;
	int num;
	char addr[DATALEN];
	char phone[DATALEN];
	char pwd[DATALEN];
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = USER_MODIFY;
	//printf("请输入您要修改职工的工号：");
	//scanf("%d",&no);
	//getchar();
	//msg->info.no = no;
	printf("+++++++++++%s++++++++++++\n",msg->username);
	printf("***********请输入要修改的选项(其他信息亲请联系管理员)*********\n");
	printf("***********	1：家庭住址   2：电话  3：密码  4：退出***********\n");
	printf("**************************************************************\n");
	printf("请输入您的选择（数字）>>");
	scanf("%d",&num);
	getchar();
	switch(num){
		case 1:
			printf("请输入新的住址：");
			scanf("%s",addr);
			getchar();
			strcpy(msg->info.addr,addr);
			//printf("%s\n",msg->info.addr);
			//strcpy(msg->recvmsg,"1");
			msg->flags = 1;
			break;
		case 2:
			printf("请输入新手机号：");
			scanf("%s",phone);
			getchar();
			strcpy(msg->info.phone,phone);
			//printf("%s\n",msg->info.phone);
			//strcpy(msg->recvmsg,"2");
			msg->flags = 2;
			break;
		case 3:
			printf("请输入新密码：");
			scanf("%s",pwd);
			getchar();
			strcpy(msg->info.passwd,pwd);
			//printf("%s\n",msg->info.passwd);
			//strcpy(msg->recvmsg,"3");
			msg->flags = 3;
			break;
		case 4:
			user_menu(sockfd,msg);
			break;
		default:
			printf("输入错误！\n");

	}
	send(sockfd , msg , sizeof(MSG) , 0);
	recv(sockfd , msg , sizeof(MSG) , 0);
	if((strncmp(msg->recvmsg,"OK",2)) == 0){
		printf("修改数据库成功\n");
		printf("修改结束\n");
	}
}

/**************************************
 *功   能：管理员菜单
 ****************************************/
void user_menu(int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	while(1)
	{
		printf("*************************************************************\n");
		printf("*************  1：查询  	2：修改		3：退出	 *************\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
			case 1:
				do_user_query(sockfd,msg);
				break;
			case 2:
				do_user_modification(sockfd,msg);
				break;
			case 3:
				msg->msgtype = QUIT;
				send(sockfd, msg, sizeof(MSG), 0);
				close(sockfd);
				exit(0);
			default:
				printf("您输入有误，请输入数字\n");
				break;
		}
	}
}


int admin_or_user_login(int sockfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	memset(msg->username, 0, NAMELEN);
	printf("请输入用户名：");
	scanf("%s",msg->username);
	getchar();
	memset(msg->passwd, 0, DATALEN);
	printf("请输入密码（6位）");
	scanf("%s",msg->passwd);
	getchar();
	//发送登陆请求
	send(sockfd, msg, sizeof(MSG), 0);
	//接受服务器响应
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("msg->recvmsg :%s\n",msg->recvmsg);

	//判断是否登陆成功
	if(strncmp(msg->recvmsg, "OK", 2) == 0)
	{
		if(msg->usertype == ADMIN)
		{
			printf("亲爱的管理员，欢迎您登陆员工管理系统！\n");
			admin_menu(sockfd,msg);
		}
		else if(msg->usertype == USER)
		{
			printf("亲爱的用户，欢迎您登陆员工管理系统！\n");
			user_menu(sockfd,msg);
		}
	}
	else
	{
		printf("登陆失败！%s\n", msg->recvmsg);
		return -1;
	}

	return 0;
}


/************************************************
 *函数名：do_login
 *参   数：套接字、消息结构体
 *返回值：是否登陆成功
 *功   能：登陆
 *************************************************/
int do_login(int sockfd)
{	
	int n;
	MSG msg;

	while(1){
		printf("*************************************************************\n");
		printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();
		switch(n)
		{
			case 1:
				msg.msgtype  = ADMIN_LOGIN;
				msg.usertype = ADMIN;
				admin_or_user_login(sockfd,&msg);
				break;
			case 2:
				msg.msgtype =  USER_LOGIN;
				msg.usertype = USER;
				admin_or_user_login(sockfd,&msg);
				break;
			case 3:
				msg.msgtype = QUIT;
				if(send(sockfd, &msg, sizeof(MSG), 0)<0)
				{
					perror("do_login send");
					return -1;
				}
				close(sockfd);
				exit(0);
			default:
				printf("您的输入有误，请重新输入\n"); 
				do_login(sockfd);
		}

		//admin_or_user_login(sockfd,&msg);
	}

}


int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes,sendbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 

	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
//	serveraddr.sin_port   = htons(5001);
//	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.200");

	if(connect(sockfd,(const struct sockaddr *)&serveraddr,addrlen) == -1){
		perror("connect failed.\n");
		exit(-1);
	}

	do_login(sockfd);

	close(sockfd);

	return 0;
}



