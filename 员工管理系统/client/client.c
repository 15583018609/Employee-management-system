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

/*{{{*/
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
	int type; 				//struct发送系信息类型
	char Name[20]; 			//姓名
	int  Age; 				//年龄
	char Sex; 				//性别
	char Password[20]; 		//密码
	char JobID[10]; 		//工号
	char Phone[16]; 		//手机号
	int  Salary; 			//工资
	char department[20]; 	//部门
}__attribute__((packed)) MSG;


//消息类型
#define REGISTER  			1   //  register  注册用户	
#define LOGIN  				2   //  login      登录（普通或超级）
		
#define RADD  				11   //  root 		超级-添加用户
#define RDELNAME  			12   //  root 		超级-删除用户-按名字
#define RDELID  			121  //  root 		超级-删除用户-按工号
#define RCHANGE  			13   //  root 		超级-更改用户信息
#define RQUERY  			14   //  root 		超级-查询所有用户信息
#define RQUERYNAME			15   //  root 		超级-查询谋个用户信息-按名字
#define RQUERYID  			16   //  root 		超级-查询谋个用户信息-按工号
#define RHISTORY  			17   //  root 		超级-查询历史记录			
			
#define UQUERY  			20   //  user 		用户-查询个人信息
#define UCHANGEPASSWORD		21   //  user 		用户-修改密码
#define UCHANGE				22   //  user 		用户-修改基本信息
#define UQUERYHISTORY  		23   //  user 		用户-历史记录

#define QUIT 				"quit quit"   //  quit退出

#define SERV_PORT 		"6666"
#define SERV_IP_ADDR 	"192.168.1.3"
#define QUITMY 			"quit"

int do_register(int sockfd, MSG *msg);
int do_login(int sockfd, MSG *msg);
int do_root(int sockfd, MSG *msg);
int do_ordinary(int sockfd, MSG *msg);
//root
int add_user(int sockfd, MSG *msg);
int del_account(int sockfd, MSG *msg);
int change_user(int sockfd, MSG *msg);
int query_all(int sockfd, MSG *msg);
int query_specific(int sockfd, MSG *msg);
int query_history(int sockfd, MSG *msg);
//ordinary
int query_personage(int sockfd,MSG *msg);
int change_password(int sockfd,MSG *msg);
int change_information(int sockfd,MSG *msg);
int query_history_user(int sockfd,MSG *msg);
/*}}}*/

char JOBID[10] = {0};
char data[256] = {0};

int main(int argc, const char *argv[])
{/*{{{*/
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
	
	//4.链接服务器
	if(connect(sockfd, (struct sockaddr*)&sin, sizeof(sin)) < 0){
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		perror("connect");
		goto ERR_NO2;
	}

	while(1){
		printf("*****************************************************************\n");
		printf("* 1.注册用户          2.登录              3.退出                 \n");
		printf("*****************************************************************\n");
		printf("Pleasese choose:");

		if(1 != scanf("%d", &num)){           	//拿到十进制输入选项
			puts("您输入错误，请重新输入");
			while(getchar() != '\n');   			//吃掉垃圾字符
			continue;
		}
		puts(""); 								//换行，显得好看
		while(getchar() != '\n');   			//吃掉垃圾字符
		
		switch(num){
		case 1: 		
			do_register(sockfd, &msg);
			break;
		case 2:
			if(do_login(sockfd, &msg) == 1){    
				if(strcmp(msg.JobID,"root") == 0){
					puts("root login");
					floag = do_root(sockfd, &msg);			//root 登录
				}else{
					puts("user login");
					floag = do_ordinary(sockfd, &msg); 		//普通登录
				}
				
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
}/*}}}*/

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
{/*{{{*/


	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = REGISTER;     //注册信号
	printf("Input Name:");
	scanf("%s", msg->Name);   //填充姓名
	while(getchar()!='\n'); 				  //吃掉回车

	printf("Input JobID:");
	scanf("%s", msg->JobID);   //填充工号
	while(getchar()!='\n'); 				  //吃掉回车

	if(strcmp(msg->JobID, "root") == 0){
		puts("非法注册！！");
		return -1;	
	}

	printf("Input Password:"); 
	scanf("%s", msg->Password);   //填充密码
	while(getchar()!='\n'); 				  //吃掉回车
	
	//在linux下写socket的程序的时候，如果尝试send到一个disconnected
	//socket上，就会让底层抛出一个SIGPIPE信号，这个信号的缺省处理方法是退出进程。
	
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
	
	
	printf("%s\n", msg->department); 
	return 0;
}/*}}}*/

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

	strcpy(JOBID, msg->JobID);   //记录谁登录了


	//在linux下写socket的程序的时候，如果尝试send到一个disconnected
	//socket上，就会让底层抛出一个SIGPIPE信号，这个信号的缺省处理方法是退出进程。
	
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
	

	if(strncmp(msg->department, "OK", 3) == 0)
	{
		printf("login ok");
		return 1;
	}else{
		printf("%s\n", msg->department);
		return 0;
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
{/*{{{*/
	puts("root login******************");
	int num = -1;
	while(1){
		printf("*****************************************************************\n");
		printf("* 1.添加用户          2.删除用户             3.更改用户信息      \n");
		printf("* 4.查询所有用户信息                         5.查询某个用户信息  \n");
		printf("* 6.查询历史记录                             7.quit              \n");
		printf("*****************************************************************\n");
		printf("Pleasese choose:");
	
		if(1 != scanf("%d", &num)){           //拿到十进制输入选项
			puts("您输入错误，请重新输入");
			while(getchar() != '\n');   			//吃掉垃圾字符
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
		case 4:
			query_all(sockfd, msg);
			break;
		case 5:
			query_specific(sockfd, msg);
			break;
		case 6:
			query_history(sockfd, msg);
			break;
		case 7:	
			return 1;
			break;
		default:
			puts("您输入选项错误，请重新输入");
			break;
		}
	}

	return 0;
}
/*}}}*/
//添加用户
int add_user(int sockfd, MSG *msg)
{/*{{{*/
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RADD;     		//注册信号
	
	printf("Input JobID:"); 
	scanf("%s", msg->JobID);   //工号
	getchar(); 	
	
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
			   
	
	printf("Input Phone:"); 
	scanf("%s", msg->Phone);   //手机号
	getchar(); 				   
	
	printf("Input Salary:"); 
	scanf("%d", &(msg->Salary));   //工资
	getchar(); 
	
	printf("Input department:"); 
	scanf("%s", msg->department);   //部门
	getchar(); 

	
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
	
	
	printf("%s\n", msg->department); 

	return 0;
}/*}}}*/
 
//删除用户
int del_account(int sockfd, MSG *msg)
{/*{{{*/
	int num = 0;
	bzero(msg, sizeof(*msg)); //清空员工信息结构体


	printf("*****************************************************************\n");
	printf("*  1.Name                    2.JobID   	                         \n");
	printf("*****************************************************************\n");
	printf("Pleasese choose:");
	
	if(1 != scanf("%d", &num)){
		printf("输入参数错误，请重新输入\n");
		while(getchar() != '\n');   			//吃掉垃圾字符
		return -1;
	}

	while(getchar() != '\n');  //吃掉多余回车

	if(num == 1){
		printf("Input Name:");
		scanf("%s", msg->Name);   //姓名
		getchar(); 				  //吃掉回车		
    	msg->type = RDELNAME;             //注册信号
		puts("按名字");
	}

	if(num == 2){
		printf("Input JobID:"); 
		scanf("%s", msg->JobID);   //工号
		getchar(); 		
    	msg->type = RDELID;             //注册信号
		puts("按工号");
	}

	printf("num = %d\n", num);
/**********************************************************************/
	
	
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}	

	printf("%s\n", msg->department); 

	return 0;
}/*}}}*/
 
//更改用户信息
int change_user(int sockfd, MSG *msg)
{/*{{{*/
	char flog = 0;
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RCHANGE;    //更改信息的宏

	printf("Input JobID:"); 
	scanf("%s", msg->JobID);   //工号
	getchar(); 			
	

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
	printf("Input Phone:"); 
	scanf("%s", msg->Phone);   //手机号
	getchar(); 				   
	printf("Input Salary:"); 
	scanf("%d", &(msg->Salary));   //工资
	getchar(); 
	printf("Input department:"); 
	scanf("%s", msg->department);   //部门
	getchar(); 

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);


	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
		

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(recv(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("recv error\n");
		return -1;
	}
	

	printf("%s\n", msg->department); 

	return 0; 
}/*}}}*/

//查询所有用户信息
int query_all(int sockfd, MSG *msg)
{/*{{{*/
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RQUERY;     		//注册信号

	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}


	bzero(data,sizeof(data));
   	printf("工号\t\t姓名\t年龄\t性别\t密码\t手机号\t\t工资\t部门\n");
   
   while(1){	
	   if(recv(sockfd, data, sizeof(data), 0) < 0)
	   {
		   printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		   printf("recv error\n");
		   return -1;
	   }

	   if(strcmp(data, QUIT) == 0)
		   break;
		printf("%s", data);
		bzero(data,sizeof(data));
	}

   return 0;
}/*}}}*/
//查询某个用户信息
int query_specific(int sockfd, MSG *msg)
{/*{{{*/
	int num = 0;
	bzero(msg, sizeof(*msg)); //清空员工信息结构体


	printf("*****************************************************************\n");
	printf("*  1.Name                    2.JobID   							 \n");
	printf("*****************************************************************\n");
	printf("Pleasese choose:");

	if(1 != scanf("%d", &num)){
		printf("输入参数错误，请重新输入\n");
		while(getchar() != '\n');   			//吃掉垃圾字符
		return -1;
	}

	while(1){
		if(num == 1){
			msg->type = RQUERYNAME;     		//注册信号
			printf("Input Name:");
			scanf("%s", msg->Name);   //姓名
			getchar();    			//吃掉回车		
			break;
		}

		if(num == 2){
			msg->type = RQUERYID;//注册信号
			printf("Input JobID:"); 
			scanf("%s", msg->JobID);   //工号
			getchar();
			break;
		}
	}  

	if((send(sockfd, msg, sizeof(*msg), 0)) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}
	while(1){
		if((recv(sockfd, msg, sizeof(*msg), 0)) < 0)
		{
			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
			printf("recv error\n");
			return -1;
		}

		if(msg->department == '\0')
			break;

		printf("工号\t姓名\t年龄\t性别\t密码\t手机号\t\t工资\t部门\n");
		printf("%s\t%s\t%d\t%c\t%s\t%s\t%d\t%s\n", 
				msg->JobID,msg->Name,msg->Age,msg->Sex,msg->Password,
				msg->Phone,msg->Salary,msg->department);
		break;
	}

	return 0;
}/*}}}*/
 //查询历史记录
int query_history(int sockfd, MSG *msg)
{/*{{{*/
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = RHISTORY;     		//注册信号

	printf("Input JobID:"); 
	scanf("%s", msg->JobID);   //工号
	getchar();

	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}

	bzero(data,sizeof(data));
   	printf("工号\t历史记录\n");
   
   while(1){	
	   if(recv(sockfd, data, sizeof(data), 0) < 0)
	   {
		   printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		   printf("recv error\n");
		   return -1;
	   }

	   if(strcmp(data, QUIT) == 0)
		   break;
		printf("%s", data);
		bzero(data,sizeof(data));
	}



	return 0;
}/*}}}*/

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
{/*{{{*/
	puts("pppppp tttttt---------------------------"); 
	int num = -1;
	while(1){
		printf("*****************************************************************\n");
		printf("* 1.查询个人信息                                      2.修改密码 \n");
		printf("* 3.修改基本信息          4.查询历史记录              5.退出     \n");
		printf("*****************************************************************\n");
		printf("Pleasese choose:");
	
		if(1 != scanf("%d", &num)){           //拿到十进制输入选项
			puts("您输入错误，请重新输入");
			while(getchar() != '\n');   			//吃掉垃圾字符
			continue;
		}
		puts(""); //换行，显得好看
		while(getchar() != '\n');   //吃掉垃圾字符
		
		switch(num){
		case 1:
			puts("普通用户查询个人信息");
			query_personage(sockfd, msg);
			puts("普通用户查询个人信息完成");
			break;
		case 2:
			puts("修改密码----");
			change_password(sockfd, msg);
			break;
		case 3:
			puts("修改基本信息----");
			change_information(sockfd, msg);
			break;
		case 4:
			query_history_user(sockfd, msg);
			break;
		case 5:	
			return 1;
			break;
		default:
			puts("您输入选项错误，请重新输入");
			break;
		}
	}
	return 0;
}/*}}}*/
//普通用户查询个人信息
int query_personage(int sockfd,MSG *msg)
{/*{{{*/
	int num = 0;
    bzero(msg, sizeof(*msg)); //清空员工信息结构体

    msg->type = UQUERY;             //注册信号
    strcpy(msg->JobID, JOBID);            

    if(send(sockfd, msg, sizeof(*msg),0) < 0)
    {   
        printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
        printf("send error\n");
        return -1;                                                                           
    }   
    bzero(msg, sizeof(*msg)); //清空员工信息结构体
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
    if(recv(sockfd, msg, sizeof(*msg),0) < 0)
    {   
        printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
        printf("recv error\n");
        return -1; 
    }   
	
    printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
//	sleep(1);

	if(strcmp(msg->department, "error") == 0){
		puts("查询出错");
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		return -1;
	}else{

		printf("工号\t姓名\t年龄\t性别\t密码\t手机号\t\t工资\t部门\n");
		printf("%s\t%s\t%d\t%c\t%s\t%s\t%d\t%s\n", 
				msg->JobID,msg->Name,msg->Age,msg->Sex,msg->Password,
				msg->Phone,msg->Salary,msg->department);

	}
	return 0;

}/*}}}*/
//修改密码   OK
int change_password(int sockfd,MSG *msg)
{/*{{{*/
	printf("登录的账户为%s\n", JOBID);   //**********************************************
	
	
	char Pass1[20] = {0};
	char Pass2[20] = {0};

	bzero(msg, sizeof(*msg)); //清空员工信息结构体
	strcpy(msg->JobID, JOBID); 		    //填充工号

	msg->type = UCHANGEPASSWORD;    //更改信息的宏

	printf("Input Old Password:"); 
	scanf("%s", msg->Password);   //密码
	getchar(); 		

	printf("Input New Password:"); 
	scanf("%s", Pass1);   //密码
	getchar();

	printf("Again Input New Password:"); 
	scanf("%s", Pass2);   //密码
	getchar(); 		

	if(strcmp(Pass1, Pass2) == 0){
		puts("输入新密码，两次结果一致"); //********************************
	}else{
		puts("New Password is unlike");
		return 2;
	}

	strcpy(msg->department, Pass1);


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
	
	if(strncmp(msg->department, "OK", 3) == 0)
	{
		printf("更改密码成功\n");
		return 1;
	}else{
		printf("%s\n", msg->department);
		return 0;
	}
}/*}}}*/

//修改基本信息  OK
int change_information(int sockfd,MSG *msg)
{/*{{{*/
	printf("登录的账户为%s\n", JOBID);   //**********************************************
	
	
	bzero(msg, sizeof(*msg)); //清空员工信息结构体
	strcpy(msg->JobID, JOBID); 		    //填充工号

	msg->type = UCHANGE;

	printf("Input Name:");
	scanf("%s", msg->Name);  //姓名
	getchar(); 				  //吃掉回车

	printf("Input Age:"); 
	scanf("%d", &(msg->Age));   //年龄
	getchar(); 				   
	
	printf("Input Sex（M:MAN  W:WOMAN）:"); 
	scanf("%c", &(msg->Sex));   //性别
	getchar(); 	   
	
	printf("Input Phone:"); 
	scanf("%s", msg->Phone);   //手机号
	getchar(); 				   
	


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


	if(strncmp(msg->department, "OK", 3) == 0)
	{
		puts("更改用户基本信息成功");
		return 1;
	}else{
		puts("更改用户基本信息失败！！！");
		return 0;
	}
}/*}}}*/

//查询历史记录
int query_history_user(int sockfd,MSG *msg)
{/*{{{*/
	bzero(msg, sizeof(*msg)); //清空员工信息结构体

	msg->type = UQUERYHISTORY;     		//注册信号
	strcpy(msg->JobID, JOBID);
	
	if(send(sockfd, msg, sizeof(*msg), 0) < 0)
	{
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		printf("send error\n");
		return -1;
	}


	bzero(data,sizeof(data));
   	printf("工号\t历史记录\n");
   
   while(1){	
	   if(recv(sockfd, data, sizeof(data), 0) < 0)
	   {
		   printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		   printf("recv error\n");
		   return -1;
	   }

	   if(strcmp(data, QUIT) == 0)
		   break;
		printf("%s", data);
		bzero(data,sizeof(data));
	}


	return 0;
}/*}}}*/
