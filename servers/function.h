#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>    
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
/************************************************************
 * Name 		: 员工管理系统服务端
 * Data 		: 2020-08-07 
 * Description 	: 普通用户和超级用户登录函数声明及结构体定义
 * Auther 		: 蒲坤
 * Version 		: V0.1
 * Modification : NO.001 ：首次编写框架代码
 *************************************************************/
#include <signal.h>
#include <time.h>

//超级管理员 账户密码： root    root123456
//消息类型
#define REGISTER  			1   //  register  注册用户	
#define LOGIN  				2   //  login      登录（普通或超级）
		
#define RADD  				11   //  root 		超级-添加用户
#define RDELNAME            12   //  root       超级-删除用户-按名字                            
#define RDELID              121  //  root       超级-删除用户-按工号
#define RCHANGE  			13   //  root 		超级-更改用户信息
#define RQUERY  			14   //  root 		超级-查询所有用户信息
#define RQUERYNAME			15   //  root 		超级-查询谋个用户信息-按名字
#define RQUERYID  			16   //  root 		超级-查询谋个用户信息-按工号
#define RHISTORY  			17   //  root 		超级-查询历史记录
			
			
#define UQUERY  			20   //  user 		用户-查询个人信息
#define UCHANGEPASSWORD		21   //  user 		用户-修改密码
#define UCHANGE				22   //  user 		用户-修改基本信息
#define UQUERYHISTORY 		23   //  user 		用户-历史记录

#define QUIT                "quit quit"   //  quit退出  

#define DATABASE "stafmanagment.db"

#define SERV_PORT       "6666"
#define SERV_IP_ADDR    "192.168.1.3" 

/*定义员工信息结构体*/
typedef struct {
	int type;			//发送系信息类型
	char Name[20];		//姓名
	int  Age;			//年龄
	char Sex;			//性别
	char Password[20];	//密码
	char JobID[10];		//工号
	char Phone[16];		//手机号	
	int  Salary;		//工资
	char department[20];//部门
}__attribute__((packed)) MSG;


int do_client(int acceptfd,sqlite3 *db);
void do_register(int acceptfd,MSG *msg,sqlite3 *db);
int do_login(int acceptfd,MSG *msg,sqlite3 *db);
int resultp(void *para,int f_num,char **f_value,char **f_name);
int change_info_person(int acceptfd,MSG *msg,sqlite3 *db);
int user_change_password(int acceptfd,MSG *msg,sqlite3 *db);
int query_specific(int acceptfd,MSG *msg,sqlite3 *db);

int query_userID_history(int acceptfd,MSG *msg,sqlite3 *db);
int callback_history(void *para,int f_num,char **f_value,char **f_name);
/********root******************/


int delete_user_Id(int acceptfd,MSG *msg,sqlite3 *db);
int delete_user_Name(int acceptfd,MSG *msg,sqlite3 *db);
int change_info_root(int acceptfd,MSG *msg,sqlite3 *db);

int add_staf_info(int acceptfd,MSG *msg,sqlite3 *db);
int query_alluser(int acceptfd,MSG *msg,sqlite3 *db);
int callback(void *para,int f_num,char **f_value,char **f_name);
int query_userID(int acceptfd,MSG *msg,sqlite3 *db);
int query_username(int acceptfd,MSG *msg,sqlite3 *db);

#endif //__FUNCTION_H__
