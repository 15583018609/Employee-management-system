/************************************************************
 * Name 		: 员工管理系统服务端
 * Data 		: 2020-08-07 
 * Description 	: 普通用户和超级用户登录功能函数
 * Auther 		: 蒲坤
 * Version 		: V0.1
 * Modification : NO.001 ：首次编写框架代码
 *************************************************************/
#include "function.h"

extern MSG msg;
extern char data[1024];

extern const char root_name[20];
extern char root_password[20];

/****************************************************************************
 *Name 				:do_client()
 *Description 		:客户端处理函数
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ******************************************************************************/
int do_client(int acceptfd,sqlite3 *db)
{
	
	while(recv(acceptfd, &msg, sizeof(MSG), 0)>0){
		switch(msg.type){
			case REGISTER:
				do_register(acceptfd, &msg, db);
				break;
			case LOGIN:
				puts("开启登录");
				do_login(acceptfd,&msg,db);
				break;
 			/*********************************************普通 命令*******************/
			case UQUERY:
				puts("查询个人信息");
				query_specific(acceptfd,&msg,db);
				break;
			case UCHANGEPASSWORD:
				puts("修改密码");
				user_change_password(acceptfd,&msg,db);
				break;
			case UCHANGE:
				puts("修改基本信息");
				change_info_person(acceptfd,&msg,db);
				break;
			case UQUERYHISTORY:
				puts("普同 用户历史记录");
				query_userID_history(acceptfd,&msg,db);
				break;
 			/**********************************************root 命令*******************/
			case RADD:
				puts("添加用户");
				add_staf_info(acceptfd,&msg,db);
				break;
			case RDELNAME:
				puts("删除用户  -按名字");
				delete_user_Name(acceptfd,&msg,db);
				break;
			case RDELID:
				puts("删除用户  -按工号");
				delete_user_Id(acceptfd,&msg,db);
				break;
			case RCHANGE:
				puts("更改用户信息");
			    printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
				change_info_root(acceptfd,&msg,db);
			    printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
				break;
			case RQUERY:
				puts("查询所有用户信息");
				query_alluser(acceptfd,&msg,db);
				break;
			case RQUERYNAME:
				puts("查询谋个用户信息-按名字");
   				query_userName(acceptfd,&msg,db);
				break;
			case RQUERYID:
				puts("查询谋个用户信息-按工号");
				query_userID(acceptfd,&msg,db);
				break;
			case RHISTORY:
				puts("查询历史记录");
				query_userID_history(acceptfd,&msg,db);
				break;

			default:
				printf("输入错误，重新输入");
		}
	}
	printf("客户端已经退出\n");
	
	close(acceptfd);
	exit(0);
}
/*****************************************************************************
 *function	    	:查询某个用户历史记录
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int query_userID_history(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char *errmsg = NULL;
	char sql[256]={0};
	int nrow = 0;
	int ncloum = 0;
	char **resultp = NULL;

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	sprintf(sql,"select * from stafmanagment_history where 工号='%s';",msg->JobID);
	printf("%s\n", sql);	
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(sqlite3_exec(db,sql,callback_history,(void *)&acceptfd,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		sqlite3_close(db);
		return 1;
	}

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	sqlite3_close(db);
	bzero(data,sizeof(data));
	strcpy(data,QUIT);
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(send(acceptfd,data,sizeof(data),0) < 0){
		 printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		 printf("recv error\n");
		 return -1;
	}
	printf("data = %s\n", data);
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);


	return 0;

}/*}}}*/


/*****************************************************************************
 *Name 				:callback_history回调函数
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int callback_history(void *para,int f_num,char **f_value,char **f_name)
{
	int acceptfd;
	acceptfd = *((int *)para);

	printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	
	bzero(data, sizeof(data));


	sprintf(data,"%s\t %s\n",f_value[0],f_value[1]);

	printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	printf("%s\n", data);
	if(send(acceptfd, data, sizeof(data), 0)<0){
		perror("send");
		printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	}

	printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);

	return 0;

}


/*****************************************************************************
 *Name 				:添加用户
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/

int add_staf_info(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char *errmsg;
	char sql[1024] = "\0";
	sprintf(sql,"insert into stafmanagment values('%s','%s',%d,'%c','%s','%s',%d,'%s')",
			msg->JobID,msg->Name,msg->Age,msg->Sex,msg->Password,msg->Phone,msg->Salary,\
			msg->department);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK){
		fprintf(stderr,"添加:%s",errmsg);
		strcpy(msg->department,"密码重复");
	}else{
		printf("添加用户信息成功！\n");
		strcpy(msg->department,"OK");
	}
	if((send(acceptfd,msg,sizeof(MSG),0))<0){
		perror("send");
        printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);  
	}
	return 1;
}/*}}}*/

/*****************************************************************************
 *function 			:查询某个用户---按名字
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int query_userName(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char *errmsg;
	char sql[256]={0};
	int nrow=0;
	int ncloum=0;
	char **resultp = NULL;
	int a =8;
	sprintf(sql,"select * from stafmanagment where 姓名='%s';",msg->Name);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloum,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		return -1;
	}
	printf("行 = %d,列 = %d\n", nrow, ncloum);
	if(1!=nrow){
		strcpy(msg->department,"error");
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	}
	if(1==nrow){
		bzero(msg,sizeof(*msg));
		strcpy(msg->JobID,resultp[a++]);
		
		if(resultp[a++] == NULL){
			strcpy(msg->Name,"");
		}else{
			strcpy(msg->Name,resultp[a-1]);
		}
		
		if(resultp[a++] == NULL){
			msg->Age = 0;
		}else{
			msg->Age=atoi((resultp[a-1]));
		}

		if(resultp[a++] == NULL){
			msg->Sex = 0;
		}else{
			msg->Sex=*(resultp[a-1]);
		}
	
 		if(resultp[a++] == NULL){
			strcpy(msg->Password,"");
		}else{
			strcpy(msg->Password,resultp[a-1]);
		}
 		 
		if(resultp[a++] == NULL){
			strcpy(msg->Phone,"");
		}else{
			strcpy(msg->Phone,resultp[a-1]);
		}
	
		if(resultp[a++] == NULL){
			msg->Salary = 0;
		}else{
			(msg->Salary)=atoi(resultp[a-1]);
		}

 		
		if(resultp[a] == NULL){
			strcpy(msg->department,"");
		}else{
			strcpy(msg->department,resultp[a]);
		}
	}
	sqlite3_free_table(resultp);
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(send(acceptfd,msg,sizeof(MSG),0) < 0){
		 printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		 printf("recv error\n");
		 return -1;
	}

	return 0;
}/*}}}*/
/***************************************************************************
 *function	    	:查询某个用户---按工号
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int query_userID(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char *errmsg;
	char sql[256]={0};
	int nrow=0;
	int ncloum=0;
	char **resultp = NULL;
	int a =8;
	sprintf(sql,"select * from stafmanagment where 工号='%s';",msg->JobID);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloum,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		return -1;
	}
	printf("行 = %d,列 = %d\n", nrow, ncloum);
	if(1!=nrow){
		strcpy(msg->department,"error");
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	}
	if(1==nrow){
		bzero(msg,sizeof(*msg));
		strcpy(msg->JobID,resultp[a++]);
		
		if(resultp[a++] == NULL){
			strcpy(msg->Name,"");
		}else{
			strcpy(msg->Name,resultp[a-1]);
		}
		
		if(resultp[a++] == NULL){
			msg->Age = 0;
		}else{
			msg->Age=atoi((resultp[a-1]));
		}

		if(resultp[a++] == NULL){
			msg->Sex = 0;
		}else{
			msg->Sex=*(resultp[a-1]);
		}
	
 		if(resultp[a++] == NULL){
			strcpy(msg->Password,"");
		}else{
			strcpy(msg->Password,resultp[a-1]);
		}
 		 
		if(resultp[a++] == NULL){
			strcpy(msg->Phone,"");
		}else{
			strcpy(msg->Phone,resultp[a-1]);
		}
	
		if(resultp[a++] == NULL){
			msg->Salary = 0;
		}else{
			(msg->Salary)=atoi(resultp[a-1]);
		}	
		if(resultp[a] == NULL){
			strcpy(msg->department,"");
		}else{
			strcpy(msg->department,resultp[a]);
		}
	}
	sqlite3_free_table(resultp);
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(send(acceptfd,msg,sizeof(MSG),0) < 0){
		 printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		 printf("recv error\n");
		 return -1;
	}

	return 0;

}/*}}}*/

/*****************************************************************************
 *Name 				:查询所有用户
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int query_alluser(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char *errmsg = NULL;
	char sql[128]={0};
	int nrow = 0;
	int ncloum = 0;
	char **resultp = NULL;

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	sprintf(sql,"select * from stafmanagment;");
	
	if(sqlite3_exec(db,sql,callback,(void *)&acceptfd,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		sqlite3_close(db);
		return 1;
	}

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	sqlite3_close(db);
	bzero(data,sizeof(data));
	strcpy(data,QUIT);
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(send(acceptfd,data,sizeof(data),0) < 0){
		 printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		 printf("recv error\n");
		 return -1;
	}
	printf("data = %s\n", data);
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
			
}/*}}}*/


/*****************************************************************************
 *Name 				:callback回调函数
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int callback(void *para,int f_num,char **f_value,char **f_name)
{
	int acceptfd;
	acceptfd = *((int *)para);

	printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	
	bzero(data, sizeof(data));

/*	sprintf(data,"%s %s %d %c %s %s %d %s\n",
				f_value[0],f_value[1], *(f_value[2]),*(f_value[3]), f_value[4],
				f_value[5],*(f_value[6]), f_value[7]);
*/	
	sprintf(data,"%s\t %s\t %d\t %c\t %s\t %s\t %d\t %s\t\n", 	
				f_value[0],f_value[1], *(f_value[2]),*(f_value[3]), f_value[4],
				f_value[5],*(f_value[6]), f_value[7]);

	printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	printf("%s\n", data);
	if(send(acceptfd, data, sizeof(data), 0)<0){
		perror("send");
		printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	}

	printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);

	return 0;

}



/*****************************************************************************
 *Name 				:修改用户信息（root用户）
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int change_info_root(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/

	char sql[256] = {0};
	char *errmsg;
	
	sprintf(sql,"update stafmanagment set 姓名='%s',年龄='%d',性别='%c',密码='%s',手机号='%s',工资='%d',部门='%s' where 工号='%s';",
			msg->Name, msg->Age, msg->Sex, msg->Password, msg->Phone, msg->Salary, msg->department, msg->JobID);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		sprintf(msg->department,"%s\n","change fail");
	}else{
	 	printf("修改基本信息成功\n");
		strcpy(msg->department,"OK");
	}
	
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	if(send(acceptfd, msg, sizeof(MSG), 0)<0){
		perror("send");
		printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	}
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 1;
}/*}}}*/
/*****************************************************************************
 *Name 				:删除用户
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int delete_user_Name(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/

	char sql[126]={0};
    char *errmsg;


	printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	sprintf(sql,"delete from stafmanagment where 姓名='%s';",msg->Name);
	
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		strcpy(msg->department,"delete fail");
	}else{
		strcpy(msg->department,"OK");
		printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
		puts("按 姓名 删除成功\n");
	}


	if(send(acceptfd, msg, sizeof(MSG), 0)<0){
		perror("send");
		printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	}

	return 0;


}/*}}}*/
int delete_user_Id(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char sql[126]={0};
    char *errmsg;

	printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	sprintf(sql,"delete from stafmanagment where 工号='%s';",msg->JobID);
	
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		strcpy(msg->department,"delete fail");
	}else{
		strcpy(msg->department,"OK");
		printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
		puts("按 工号 删除成功\n");
	}


	if(send(acceptfd, msg, sizeof(MSG), 0)<0){
		perror("send");
		printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	}

	return 0;

}/*}}}*/

/*****************************************************************************
 *Name 				:修改密码
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int user_change_password(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char sql[256] = {0};
	char *errmsg;
	int nrow;
	int ncloum;
	char **resultp;
	sprintf(sql, "select * from stafmanagment where 工号 = '%s' and 密码 = '%s';",msg->JobID,msg->Password);
	
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloum,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);     
	}else{
		printf("成功查询到该用户!\n");
	}

	if(nrow == 1){
		sprintf(sql,"update stafmanagment set 密码='%s' where 工号='%s';",msg->department, msg->JobID);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
			printf("%s\n",errmsg);
		}else{
			printf("修改密码成功\n");

			strcpy(msg->department,"OK");
		}

		if(send(acceptfd, msg, sizeof(MSG), 0)<0){
			perror("send");
			printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
		}

		return 0;	

	}else{
		return -1;
	}
	
}/*}}}*/

/*****************************************************************************
 *Name 				:修改个人信息（姓名，性别，年龄，手机号）
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int change_info_person(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char sql[256] = {0};
	char *errmsg;

	sprintf(sql,"update stafmanagment set 姓名='%s',年龄='%d',性别='%c',手机号='%s' where 工号='%s';",msg->Name, msg->Age, msg->Sex, msg->Phone, msg->JobID);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
	}else{
	 	printf("修改基本信息成功\n");

		strcpy(msg->department,"OK");
	 }
	
	if(send(acceptfd, msg, sizeof(MSG), 0)<0){
		perror("send");
		printf("%s:%s:%d\n", __FILE__,__func__,__LINE__);
	}

	return 0;	
}/*}}}*/

/*****************************************************************************
 *Name 				:do_register()
 *Description 		:用户注册函数
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
void do_register(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char *errmsg;
	char sql[128] = {0};
//	bzero(msg, sizeof(MSG));
//	sprintf(sql,"insert into stafmanagment (工号,姓名,密码) values('%s','%s','%s');",msg->JobID,msg->Name,msg->Password);
	sprintf(sql,"insert into stafmanagment values('%s','%s',0,'N','%s','00000000000',0,'N');",
			msg->JobID,msg->Name,msg->Password);
	//sprintf(sql,"insert into stafmanagment values('%s','%s',%d,'%c','%s','%s',%d,'%s')",
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		perror("sqlite3_exec");
		strcpy(msg->department,"error \n");
	 }else{
	 	printf("注册成功\n");

		//select max(field1) from table1;查询最大键值


		strcpy(msg->department,"register success!");
	 }
	
	if(send(acceptfd, msg, sizeof(MSG), 0)<0){
		perror("send");
		exit(1);
	}

}/*}}}*/
/*****************************************************************************
 *Name 				:do_login()
 *Description 		:用户登录函数
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int do_login(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
	char sql[128] = {0};
	char *errmsg;
	int nrow;
	int ncloum;
	char **resultp;
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(strcmp(msg->JobID, root_name) == 0){
		if(strcmp(msg->Password, root_password) == 0){       //root 用户登录成功头

			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);

			strcpy(msg->department,"OK");
			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
			if(send(acceptfd,msg,sizeof(MSG),0) < 0){
				printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
				printf("recv error\n");
				return -1;
			}
			puts("root 登录成功");
		}else{
			strcpy(msg->department,"Password error");
			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
			if(send(acceptfd,msg,sizeof(MSG),0) < 0){
				printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
				printf("recv error\n");
				return -1;
			}
			puts("root 密码 错误");
		}
	}else{ 												//root 用户登录失败，执行普通用户登录

		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);

		sprintf(sql, "select * from stafmanagment where 工号 = '%s' and 密码 = '%s';",msg->JobID,msg->Password);

		if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloum,&errmsg)!= SQLITE_OK){
			printf("%s\n",errmsg);
			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);     
		}else{
			printf("成功查询到%d个该用户!\n", nrow);
		}

		if(nrow == 1){
			strcpy(msg->department,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}

		if(nrow == 0){
			strcpy(msg->department,"login error\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}

	}

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);     
	time_t t;
	time(&t);

	struct tm *tp;
	//进行时间格式转换
	tp = localtime(&t);	
	
	char time_c[128] = {0};
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);     
	
	sprintf(time_c, "%d-%d-%d %d:%d:%d", tp->tm_year + 1900, tp->tm_mon+1, tp->tm_mday, 
			tp->tm_hour, tp->tm_min, tp->tm_sec);
	printf("get data:%s\n", time_c);
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	sprintf(sql,"insert into stafmanagment_history values('%s','%s');",msg->JobID, time_c);
	printf("%s\n", sql);

	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);     
	
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK){
		fprintf(stderr,"插入:%s\n",errmsg);
		return 1;
	}else
		puts("***插入历史记录成功！");
	return 0;

	
	

}/*}}}*/


/*****************************************************************************
 *Name 				:查询个人信息
 *Auther 			:蒲坤
 *time 				:2020-8-8
 ****************************************************************************/
int query_specific(int acceptfd,MSG *msg,sqlite3 *db)
{/*{{{*/
    char sql[256]= {0};
    char **resultp = NULL;
    char *errmsg;
    int nrow;
    int ncloum;
    int a = 8;
   // sqlite3_free_table(resultp);
    
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	sprintf(sql,"select * from stafmanagment where 工号='%s';",msg->JobID);
//	sprintf(sql,"SELECT * FROM stafmanagment WHERE 工号='11';");
    if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloum,&errmsg)!= SQLITE_OK){
        printf("%s\n",errmsg);
        return -1;
    }
	printf("行 = %d,列 = %d\n", nrow, ncloum);
    if(1!=nrow){
        strcpy(msg->department,"error");
		printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);

    }
    if(1 == nrow){
		bzero(msg, sizeof(*msg));
        strcpy(msg->JobID,resultp[a++]);

		if(resultp[a++] == NULL){
			strcpy(msg->Name,"");
		}else{
			strcpy(msg->Name,resultp[a-1]);
		}


		if(resultp[a++] == NULL){
			msg->Age = 0;	
		}else{
			msg->Age = atoi((resultp[a-1]));	
		}

		if(resultp[a++] == NULL){
			msg->Sex = 0;  
		}else{
			msg->Sex = *(resultp[a-1]);  
		}

		if(resultp[a++] == NULL){
			strcpy(msg->Password,"");
		}else{
			strcpy(msg->Password,resultp[a-1]);
		}
 
		if(resultp[a++] == NULL){
			strcpy(msg->Phone,"");
		}else{
			strcpy(msg->Phone,resultp[a-1]);
		}
	
		if(resultp[a++] == NULL){
			msg->Salary = 0;
			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		}else{
			(msg->Salary) = atoi(resultp[a-1]);
			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		}
		if(resultp[a] == NULL){
			strcpy(msg->department, "noy have");
			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		}else{
			strcpy(msg->department,resultp[a]);
			printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
		}

	   printf("工号\t姓名\t年龄\t性别\t密码\t手机号\t工资\t部门\n");
		printf("%s\t%s\t%d\t%c\t%s\t%s\t%d\t%s\n",                             
				msg->JobID,msg->Name,msg->Age,msg->Sex,msg->Password,
				msg->Phone,msg->Salary,msg->department);
    }

    sqlite3_free_table(resultp);
   
	printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
    if(send(acceptfd,msg,sizeof(MSG),0) < 0){
        printf("%s:%s:%d\n",__FILE__,__func__,__LINE__);
        printf("recv error\n");
        return -1;
    }
puts("fawanle");
    return 0;

}  /*}}}*/


