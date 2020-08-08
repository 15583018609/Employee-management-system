/************************************************************
 * Name 		: 员工管理系统客户端
 * Data 		: 2020-08-07 
 * Description 	: 普通用户和超级用户登录
 * Auther 		: 朱德茂
 * Version 		: V0.1
 * Modification : NO.001 ：首次编写框架代码
 * 							
 *
 *************************************************************/


#include <stdio.h>
#include <sys/types.h>      
#include <sys/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>


/*定义员工信息结构体*/
typedef struct {
	int type;//struct发送系信息类型
	char Name[20];//姓名
	int  Age;//年龄
	char Sex;//性别
	char Password[20];//密码
	char JobID[10];//å·¥号
	char Phone[16];//手机号
	int  Salary;//工资
	char department[20];//部门
}__attribute__((packed)) MSG;


//消息类型
#define REGISTER  			1   //  register  注册用户	
#define LOGIN  				2   //  login      登录（普通或超级）
		
#define RADD  				11   //  root 		超级-添加用户
#define RDEL  				12   //  root 		超级-删除用户
#define RCHANGE  			13   //  root 		超级-更改用户信息
#define RQUERY  			14   //  root 		超级-查询所有用户信息
#define RQUERYNAME			15   //  root 		超级-查询谋个用户信息-按名字
#define RQUERYID  			16   //  root 		超级-查询谋个用户信息-按工号
#define RHISTORY  			17   //  root 		超级-查询历史记录			
			
#define UQUERY  			20   //  user 		用户-查询个人信息
#define UCHANGEPASSWORD		21   //  user 		用户-修改密码
#define UCHANGE				22   //  user 		用户-修改基本信息
#define UQUERYHISTORY  		23   //  user 		用户-历史记录

#define QUIT 				30   //  quit退出

#define SERV_PORT 		"6666"
#define SERV_IP_ADDR 	"192.168.0.3"
#define QUITMY 			"quit"

int do_register(int sockfd, MSG *msg);
int do_login(int sockfd, MSG *msg);
int do_root(int sockfd, MSG *msg);
int do_ordinary(int sockfd, MSG *msg);

int add_user(int sockfd, MSG *msg);
int del_account(int sockfd, MSG *msg);
int change_user(int sockfd, MSG *msg);


int main(int argc, const char *argv[])
{
	int num;
	MSG msg;
	int floag = 0;

	//1.创建通信套接子
	int sockfd = -1;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		perror("creation socket error");
		goto ERR_NO1;
	}

	/*链接服务器阶段*/
	//2.创建通信结构体
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin));

	//3.填充通信信息结构体
	sin.sin_family 		= AF_INET;
	sin.sin_port 		= htons(atoi(SERV_PORT)); //转数字//转网络字节序
	if(inet_pton(AF_INET, SERV_IP_ADDR, (void*)&sin.sin_addr.s_addr) != 1){
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		perror("inet_pton");
		goto ERR_NO2;
	}
	
#if 0
	//4.链接服务器
	if(connect(sockfd, (struct sockaddr*)&sin, sizeof(sin)) < 0){
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		perror("connect");
		goto ERR_NO2;
	}
	puts("到此链接成功");
#endif
	while(1){
		printf("*****************************************************************\n");
		printf("* 1.register          2.login              3.quit                \n");
		printf("*****************************************************************\n");
		printf("Pleasese choose:");

		if(1 != scanf("%d", &num)){           //拿到十进制输入选项
			puts("您输入错误，请重新输入");
			continue;
		}
		puts(""); //换行，显得好看
		while(getchar() != '\n');   //吃掉垃圾字符
		
		switch(num){
		case 1: 		
			do_register(sockfd, &msg);
			break;
		case 2:
			if(do_login(sockfd, &msg) == 0){    //正常为1 验证写的0   ******************************
				if(strcmp(msg.JobID,"root") == 0){
					floag = do_root(sockfd, &msg);			//root 登录
				}else{
					floag = do_ordinary(sockfd, &msg); 		//普通登录
				}
				
			}else{
				puts("登录失败");
			}
			break;
		case 3:	
			close(sockfd);
			return 0;
			break;
		default:
			puts("您输入选项错误，请重新输入");
			break;
		}


		if(floag == 1){    //如果floag 为1 就退出程序  
			break;
		}
	}

	close(sockfd);
	return 0;

ERR_NO2:
	close(sockfd);
ERR_NO1:
	return -1;
}

/************************************************************
 * Name 		: do_register	
 * Description 	: 注册用户
 * Inouts 		: 
 * 		a : sockfd
 * 		b : MSG *
 * Output 		: 
 * 		成功：0  
 * 		失败：-1
 * Auther 		: 朱德茂
 * Other 		: 无
 *
*************************************************************/
int do_register(int sockfd, MSG *msg)
{
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = REGISTER;     //注册信号
	printf("Input JobID:");
	scanf("%s", msg->JobID);   //填充姓名
	getchar(); 				  //吃掉回车

	printf("Input Password:"); 
	scanf("%s", msg->Password);   //填充密码
	getchar(); 				      //吃掉回车
	
	//在linux下写socket的程序的时候，如果尝试send到一个disconnected
	//socket上，就会让底层抛出一个SIGPIPE信号，这个信号的缺省处理方法是退出进程。
	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}
	
	*/
	printf("%s\n", msg->department); 
	return 0;
}


/************************************************************
 * Name 		: 登录	
 * Description 	: 登录服务器
 * Inouts 		: 
 * 					a : sockfd 
 * 					b : msg
 * Output 		:
 * 					成功：1
 * 					失败：0
 * Auther 		: 朱德茂 
 * Other 		: 无
 *
*************************************************************/
int do_login(int sockfd, MSG *msg)
{/*{{{*/

	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = LOGIN;     		//登录信号
	printf("Input JobID:");
	scanf("%s", msg->JobID);   //填充工号
	getchar(); 				  //吃掉回车

	printf("Input Password:"); 
	scanf("%s", msg->Password);   //填充密码
	getchar(); 				      //吃掉回车
	
	//在linux下写socket的程序的时候，如果尝试send到一个disconnected
	//socket上，就会让底层抛出一个SIGPIPE信号，这个信号的缺省处理方法是退出进程。
	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}
	*/

	if(strncmp(msg->department, "OK", 3) == 0)
	{
		printf("login ok");
		return 1;
	}else{
		printf("%s\n", msg->department);
	}

	return 0;
}/*}}}*/


/************************************************************
 * Name 		: do_root
 * Description 	: root 用户 登录处理函数 
 * Inouts 		: //输入参数描述
 * 		a : sockfd 
 * 		b : MSG *
 * Output 		: //输出参数描述
 * 		
 * Auther 		: 朱德茂 
 * Other 		: 
 *
*************************************************************/
int do_root(int sockfd, MSG *msg)
{
	puts("root login******************");
	int num = -1;
	while(1){
		printf("*****************************************************************\n");
		printf("* 1.add_user          2.del_account              3.change_user   \n");
		printf("* 4.Query all information 		   5.Query specific information  \n");
		printf("* 6.Query history       						 7.quit          \n");
		printf("*****************************************************************\n");
		printf("Pleasese choose:");
	
		if(1 != scanf("%d", &num)){           //拿到十进制输入选项
			puts("您输入错误，请重新输入");
			continue;
		}
		puts(""); //换行，显得好看
		while(getchar() != '\n');   //吃掉垃圾字符
		
		switch(num){
		case 1: 		
			add_user(sockfd, msg);
			break;
		case 2:
			del_account(sockfd, msg);
			break;
		case 3:
			change_user(sockfd, msg);
			break;
		case 7:	
			close(sockfd);
			return 1;
			break;
		default:
			puts("您输入选项错误，请重新输入");
			break;
		}
	}

	return 0;
}
/*{{{*/
/************************************************************
 * Name 		: do_ordinary
 * Description 	: 普通登录处理函数 
 * Inouts 		: //输入参数描述
 * 		a : sockfd 
 * 		b : MSG *
 * Output 		: //输出参数描述
 * 		
 * Auther 		: 朱德茂
 * Other 		: 
 *
*************************************************************/
int do_ordinary(int sockfd, MSG *msg)
{
	puts("pppppp tttttt---------------------------"); 
	return 0;
}
/*}}}*/
//添加用户
int add_user(int sockfd, MSG *msg)
{/*{{{*/
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RADD;     		//注册信号
	printf("Input Name:");
	scanf("%s", msg->Name);  //姓名
	getchar(); 				  //吃掉回车

	printf("Input Age:"); 
	scanf("%d", &(msg->Age));   //年龄
	getchar(); 				   
	
	printf("Input Sex（M:MAN  W:WOMAN）:"); 
	scanf("%c", &(msg->Sex));   //性别
	getchar(); 				   
	
	printf("Input Password:"); 
	scanf("%s", msg->Password);   //密码
	getchar(); 				   

	printf("Input JobID:"); 
	scanf("%s", msg->JobID);   //工号
	getchar(); 				   
	
	printf("Input Phone:"); 
	scanf("%s", msg->Phone);   //手机号
	getchar(); 				   
	
	printf("Input Salary:"); 
	scanf("%d", &(msg->Salary));   //工资
	getchar(); 
	
	printf("Input department:"); 
	scanf("%s", msg->department);   //部门
	getchar(); 

	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}
	
	*/
	printf("%s\n", msg->department); 

	return 0;
}/*}}}*/
 
//删除用户
int del_account(int sockfd, MSG *msg)
{/*{{{*/
	int num;
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RDEL;     		//注册信号

	printf("*****************************************************************\n");
	printf("*  1.Name                    2.JobID   							 \n");
	printf("*****************************************************************\n");
	
	if(1 != scanf("%d", &num)){
		printf("输入参数错误，请重新输入\n");
		return -1;
	}

	while(getchar() != '\n');  //吃掉多余回车

	if(num == 1){
		printf("Input Name:");
		scanf("%s", msg->Name);   //姓名
		getchar(); 				  //吃掉回车		
	}

	if(num == 2){
		printf("Input JobID:"); 
		scanf("%s", msg->JobID);   //工号
		getchar(); 		
	}

	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}	
	*/
	printf("%s\n", msg->department); 
	printf("num = %d\n", num);

	return 0;
}/*}}}*/
 
//更改用户信息
int change_user(int sockfd, MSG *msg)
{
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RCHANGE;    //更改信息的宏

	printf("Input Name:");
	scanf("%s", msg->Name);  //姓名
	getchar(); 				  //吃掉回车

	printf("Input Age:"); 
	scanf("%d", &(msg->Age));   //年龄
	getchar(); 				   
	
	printf("Input Sex（M:MAN  W:WOMAN）:"); 
	scanf("%c", &(msg->Sex));   //性别
	getchar(); 				   
	
	printf("Input Password:"); 
	scanf("%s", msg->Password);   //密码
	getchar(); 				   

	printf("Input JobID:"); 
	scanf("%s", msg->JobID);   //工号
	getchar(); 				   
	
	printf("Input Phone:"); 
	scanf("%s", msg->Phone);   //手机号
	getchar(); 				   
	
	printf("Input Salary:"); 
	scanf("%d", &(msg->Salary));   //工资
	getchar(); 
	
	printf("Input department:"); 
	scanf("%s", msg->department);   //部门
	getchar(); 

	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}
	
	*/
	printf("%s\n", msg->department); 

	return 0; 
}

//查询所有用户信息
int del_account(int sockfd, MSG *msg)
{/*{{{*/
	int num;
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RDEL;     		//注册信号

	printf("*****************************************************************\n");
	printf("*  1.Name                    2.JobID   							 \n");
	printf("*****************************************************************\n");
	
	if(1 != scanf("%d", &num)){
		printf("输入参数错误，请重新输入\n");
		return -1;
	}

	while(getchar() != '\n');  //吃掉多余回车

	if(num == 1){
		printf("Input Name:");
		scanf("%s", msg->Name);   //姓名
		getchar(); 				  //吃掉回车		
	}

	if(num == 2){
		printf("Input JobID:"); 
		scanf("%s", msg->JobID);   //工号
		getchar(); 		
	}

	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}	
	*/
	printf("%s\n", msg->department); 
	printf("num = %d\n", num);

	return 0;
}/*}}}*/
//查询所有用户信息
int del_account(int sockfd, MSG *msg)
{/*{{{*/
	int num;
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RDEL;     		//注册信号

	printf("*****************************************************************\n");
	printf("*  1.Name                    2.JobID   							 \n");
	printf("*****************************************************************\n");
	
	if(1 != scanf("%d", &num)){
		printf("输入参数错误，请重新输入\n");
		return -1;
	}

	while(getchar() != '\n');  //吃掉多余回车

	if(num == 1){
		printf("Input Name:");
		scanf("%s", msg->Name);   //姓名
		getchar(); 				  //吃掉回车		
	}

	if(num == 2){
		printf("Input JobID:"); 
		scanf("%s", msg->JobID);   //工号
		getchar(); 		
	}

	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}	
	*/
	printf("%s\n", msg->department); 
	printf("num = %d\n", num);

	return 0;
}/*}}}*/
 //查询所有用户信息
int del_account(int sockfd, MSG *msg)
{/*{{{*/
	int num;
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RDEL;     		//注册信号

	printf("*****************************************************************\n");
	printf("*  1.Name                    2.JobID   							 \n");
	printf("*****************************************************************\n");
	
	if(1 != scanf("%d", &num)){
		printf("输入参数错误，请重新输入\n");
		return -1;
	}

	while(getchar() != '\n');  //吃掉多余回车

	if(num == 1){
		printf("Input Name:");
		scanf("%s", msg->Name);   //姓名
		getchar(); 				  //吃掉回车		
	}

	if(num == 2){
		printf("Input JobID:"); 
		scanf("%s", msg->JobID);   //工号
		getchar(); 		
	}

	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}	
	*/
	printf("%s\n", msg->department); 
	printf("num = %d\n", num);

	return 0;
}/*}}}*/
 
//查询所有用户信息
int del_account(int sockfd, MSG *msg)
{/*{{{*/
	int num;
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RDEL;     		//注册信号

	printf("*****************************************************************\n");
	printf("*  1.Name                    2.JobID   							 \n");
	printf("*****************************************************************\n");
	
	if(1 != scanf("%d", &num)){
		printf("输入参数错误，请重新输入\n");
		return -1;
	}

	while(getchar() != '\n');  //吃掉多余回车

	if(num == 1){
		printf("Input Name:");
		scanf("%s", msg->Name);   //姓名
		getchar(); 				  //吃掉回车		
	}

	if(num == 2){
		printf("Input JobID:"); 
		scanf("%s", msg->JobID);   //工号
		getchar(); 		
	}

	/*
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}	
	*/
	printf("%s\n", msg->department); 
	printf("num = %d\n", num);

	return 0;
}/*}}}*/
 


