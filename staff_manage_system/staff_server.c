/*************************************************************************
#	 FileName	: server.c
#	 Author		: ZhangGe
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

void do_query(int acceptfd,MSG* msg)
{
    int i = 0,j = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;
    if(msg->flags == 1){
	   sprintf(sql,"select * from usrinfo where name='%s';",msg->username);
	}else{
	   sprintf(sql,"select * from usrinfo");
	}
	if(sqlite3_get_table(db, sql, &resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("searching.....\n");	
		for(i = 0; i < ncolumn; i ++){
			printf("%-8s ",resultp[i]);
		}
		printf("row = %d column = %d\n",nrow,ncolumn);
		puts("");
		puts("======================================================================================");
				
		int index = ncolumn;
		for(i = 0; i < nrow; i ++){
			printf("%s    %s     %s     %s     %s     %s     %s     %s     %s     %s     %s.\n",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
				
			sprintf(msg->recvmsg,"%s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s;",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			send(acceptfd,msg,sizeof(MSG),0);
			
			usleep(1000);
			puts("======================================================================================");
			index += ncolumn;
		}
		memset(msg->recvmsg,0,sizeof(msg->recvmsg));
		strcpy(msg->recvmsg,"over");
		send(acceptfd,msg,sizeof(MSG),0);

		sqlite3_free_table(resultp);
		printf("sqlite3_get_table successfully.\n");
	}

}


int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;

	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",
			 msg->info.usertype,msg->info.name ,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	return 0;	
}

int process_user_modify_request(int acceptfd,MSG *msg)
{
    process_admin_modify_request(acceptfd,msg);
}



int process_user_query_request(int acceptfd,MSG *msg)
{
	do_query(acceptfd,msg);
}


int process_admin_modify_request(int acceptfd,MSG *msg)
{
    char sql[128]={0};
	char *errmsg;

	switch(msg->recvmsg[0])
	{
	   case '1':
	   sprintf(sql,"update usrinfo set name='%s' where staffno=%d",
			   msg->info.name,msg->info.no);break;
	   case '2':
	   sprintf(sql,"update usrinfo set age=%d where staffno=%d",
			   msg->info.age,msg->info.no);break;
	   case '3':
	   sprintf(sql,"update usrinfo set addr='%s' where staffno=%d",
			   msg->info.addr,msg->info.no);break;
	   case '4':
	   sprintf(sql,"update usrinfo set phone='%s' where staffno=%d",
			   msg->info.phone,msg->info.no);break;
	   case '5':
	   sprintf(sql,"update usrinfo set work='%s' where staffno=%d",
			   msg->info.work,msg->info.no);break;
	   case '6':
	   sprintf(sql,"update usrinfo set salary=%lf where staffno=%d",
			   msg->info.salary,msg->info.no);break; 
	   case '7':
	   sprintf(sql,"update usrinfo set date='%s' where staffno=%d",
			   msg->info.date,msg->info.no);break;
	   case '8':
	   sprintf(sql,"update usrinfo set level=%d where staffno=%d",
			   msg->info.level,msg->info.no);break;
	   case '9':
	   sprintf(sql,"update usrinfo set passwd='%s' where staffno=%d",
			   msg->info.passwd,msg->info.no);break;

	}


	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{	
		printf("%s\n",errmsg);
		return;
	}
	strcpy(msg->recvmsg,"OK");
	send(acceptfd,msg,sizeof(msg),0);
	puts("update ok!");
	return;
	//printf("------------%s-----------%d.\n",__func__,__LINE__);

     
}


int process_admin_adduser_request(int acceptfd,MSG *msg)
{
  
       char sql[128]={0};
	   char *errmsg;
	   sprintf(sql,"insert into usrinfo values(%d,%d,'%s','%s',%d,'%s','%s','%s','%s',%d,%f)",
           msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);//拼接SQL语句
	   if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	   {
	     printf("%s\n",errmsg);
	     return;
	   }
	   strcpy(msg->recvmsg,"ok");
	   send(acceptfd,msg,sizeof(MSG),0);
	   puts("insert ok!");
	  return;
}



int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
    char sql[128]={0};
	char *errmsg;
	
	sprintf(sql,"delete from usrinfo where staffno =%d and name =%s",msg->info.no,msg->info.name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return;
	}
	strcpy(msg->recvmsg,"OK");
	send(acceptfd,msg,sizeof(MSG),0);
	puts("delete ok!");
	return;
}


int process_admin_query_request(int acceptfd,MSG *msg)
{
    do_query(acceptfd,msg); 
}
void get_time(char *date)//获得时间
{
	time_t mytime;
	struct tm *mytm;
	mytime=time(NULL);//得到秒数
	mytm=localtime(&mytime);//得到当前的时间
	sprintf(date,"%04d-%02d-%02d  %02d:%02d:%02d",mytm->tm_year+1900,mytm->tm_mon+1,mytm->tm_mday,\
			mytm->tm_hour,mytm->tm_min,mytm->tm_sec);
}

void insert_history(MSG *msg,char* words)
{
	char sql[128]={0};
	char *errmsg;
	char date[64]={0};
	get_time(date);//获得当前的日期
	sprintf(sql,"insert into historyinfo values('%s','%s','%s')",date,msg->username,words);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return;
	}
}

int process_admin_history_request(int acceptfd,MSG *msg)
{
	char sql[128]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	int i;
	msg->flags = 0;
	sprintf(sql,"select * from historyinfo;");
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg)!=SQLITE_OK)	{
		printf("---****----%s.\n",errmsg);		
	}else{
		int num=ncolumn;
		//printf("num = %d\n",num);
		for(i=0;i<nrow;i++){
			strcpy(msg->info.date,result[num]);
			strcpy(msg->username,(char *)result[num+1]);
			strcpy(msg->recvmsg,(char *)result[num+2]);
			send(acceptfd,msg,sizeof(MSG),0);
			num=num+3;
		}
		msg->flags = 1;
		send(acceptfd,msg,sizeof(MSG),0);
		memset(msg->recvmsg,0,sizeof(msg->recvmsg));
	}
	return 0;
}


int process_client_quit_request(int acceptfd,MSG *msg)
{
	//printf("------------%s-----------%d.\n",__func__,__LINE__);
	close(acceptfd);

}


int process_client_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(acceptfd,msg);
			insert_history(msg,"LOGIN");
			break;
		case USER_MODIFY:	
			process_user_modify_request(acceptfd,msg);
			insert_history(msg,"USER_MODIFY");
			break;
		case USER_QUERY:
			process_user_query_request(acceptfd,msg);
			insert_history(msg,"USER_QUERY");
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(acceptfd,msg);
			insert_history(msg,"ADMIN_MODIFY");
			break;

		case ADMIN_ADDUSER:
			process_admin_adduser_request(acceptfd,msg);
			insert_history(msg,"ADMIN_ADDUSER");
			break;

		case ADMIN_DELUSER:
			process_admin_deluser_request(acceptfd,msg);
			insert_history(msg,"ADMIN_DELUSER");
			break;
		case ADMIN_QUERY:
			process_admin_query_request(acceptfd,msg);
			insert_history(msg,"ADMIN_QUERY");
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(acceptfd,msg);
			insert_history(msg,"ADMIN_HISTORY");
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
	int serverFd;
	int acceptFd;
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
	}else{ 
		printf("create historyinfo table success.\n");
	}

	//创建网络通信的套接字
	serverFd = socket(AF_INET,SOCK_STREAM, 0);
	if(serverFd == -1){
		perror("socket failed.\n");
		return -1;
	}
	//printf("sockfd :%d.\n",serverFd); 

	
	/*优化4： 允许绑定地址快速重用 */
	int b_reuse = 1;
	setsockopt (serverFd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));
	
	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
//	serveraddr.sin_port   = htons(atoi(argv[2]));
//	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port   = htons(5001);
	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.33");


	//绑定网络套接字和网络结构体
	if(bind(serverFd, (const struct sockaddr *)&serveraddr,addrlen) < 0){
		printf("bind failed.\n");
		return -1;
	}

	//监听套接字，将主动套接字转化为被动套接字
	if(listen(serverFd,10) < 0){
		printf("listen failed.\n");
		return -1;
	}

	//定义一张表
	fd_set readfds,tempfds;
	//清空表
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	//添加要监听的事件
	FD_SET(serverFd,&readfds);
	int maxfd = serverFd+1;
	int retval;
	int i = 0;

	while(1){
		tempfds = readfds;
		//记得重新添加
		retval =select(maxfd, &tempfds, NULL,NULL,NULL);
		//判断是否是集合里关注的事件
		for(i = 0;i < maxfd; i ++){
			if(FD_ISSET(i,&tempfds)){
				if(i == serverFd){
					//数据交互 
					acceptFd = accept(serverFd,(struct sockaddr *)&clientaddr,&cli_len);
					if(acceptFd == -1){
						printf("acceptfd failed.\n");
						return -1;
					}
					//printf("ip : %s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptFd,&readfds);
				    if(acceptFd > maxfd-1){
						maxfd = acceptFd + 1;
					}
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
	close(serverFd);

	return 0;
}










