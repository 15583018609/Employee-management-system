/************************************************************
 * Name 		: 员工管理系统服务端
 * Data 		: 2020-08-07 
 * Description 	: 服务端主函数
 * Auther 		: 蒲坤
 * Version 		: V0.1
 * Modification : NO.001 ：首次编写框架代码
 *************************************************************/
#include "function.h"

MSG msg;
char data[256] = {0};

const char root_name[20] = "root";
char root_password[20] = "123";


int main(int argc, const char *argv[])
{
	int sockfd;
	int acceptfd = 0;
	sqlite3 *db=NULL;
	pid_t pid;
	char *errmsg;
	struct sockaddr_in serveraddr;
/*
	if(argc != 3){
		printf("Usage:%s server port.\n",argv[0]);
		exit(1);
	}
*/
	if(sqlite3_open(DATABASE,&db)!= SQLITE_OK){
		perror("sqlite3_open error\n");
		exit(1);
	}
	//autoincrement,整数递增	
	char *create_table = "CREATE TABLE if not exists \
	stafmanagment('工号' char primary key, '姓名'char ,'年龄' int, '性别'\
	char ,'密码' char,'手机号' char,'工资' int,'部门' char );";
	if(sqlite3_exec(db,create_table,NULL,NULL,&errmsg)!= SQLITE_OK ){
			printf("%s\n",errmsg);
			sqlite3_close(db);
			return -1;
	}

	printf("员工管理信息表创建成功!\n");

	char *create_table_history = "CREATE TABLE if not exists \
	stafmanagment_history('工号' TEXT,'记录' TEXT);";
	if(sqlite3_exec(db,create_table_history,NULL,NULL,&errmsg)!= SQLITE_OK ){
			printf("%s\n",errmsg);
			sqlite3_close(db);
			return -1;
	}

	printf("员工登录历史记录表创建完成!\n");


	sockfd =  socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd <0){
		perror("创建流式套接字失败");
		exit(1);
	}
	
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family      = AF_INET;
//	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
//	serveraddr.sin_port 	   = htons(atoi(argv[2])); 
	serveraddr.sin_addr.s_addr = inet_addr(SERV_IP_ADDR);
	serveraddr.sin_port 	   = htons(atoi(SERV_PORT)); 

	if( bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0){
		perror("绑定IP地址失败\n");
		exit(1);
	}

	if(listen(sockfd, 9)<0){
		perror("监听套接字监听失败");
		exit(1);
	}

	signal(SIGCHLD,SIG_IGN);	

	while(1){
		acceptfd = accept(sockfd,NULL,NULL);
		if(acceptfd<0){
			perror("接受失败\n");
			exit(1);
		}
		
		pid = fork();

		if(pid<0){
			perror("fork失败\n");
			exit(1);
		}else if(pid >0){
			close(acceptfd);
			puts("父回去继续接待");
		}else{
			close(sockfd);
			puts("子进程开始干活");
			do_client(acceptfd,db);
			return 0;
		}
	}
	return 0;
}

