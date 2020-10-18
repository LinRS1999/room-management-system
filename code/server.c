#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<pthread.h>
#include<unistd.h>
#include<mysql/mysql.h>
void* communication(void* arg);
int main()
{
	int sockfd=socket(PF_INET,SOCK_STREAM,0);  //创建套接字
	assert(-1!=sockfd);   //创建失败返回错误
	struct sockaddr_in ser,cli;  //套接字地址
	ser.sin_family=AF_INET;  //定义地址域为IPv4
	ser.sin_port=htons(3000);   //设置端口号
	ser.sin_addr.s_addr=htonl(INADDR_ANY);  //定义套接字地址
	int res=bind(sockfd,(struct sockaddr*)&ser,sizeof(ser));  //绑定套接字与设置的端口
	assert(-1!=res);  //绑定失败返回错误
	listen(sockfd,5);  //设置监听，最大连接请求为5
	while(1)//保证服务器可以连接多个客户端
	{
		int len=sizeof(cli);
		int c=accept(sockfd,(struct sockaddr*)&cli,&len);  //接受客户端的连接请求
        if(c<0)
        {
            printf("link error\n");
            continue;
        }
		pthread_t id;
		int n=pthread_create(&id,NULL,communication,(void*)(long)c);//创建线程，将文件描述符强转为void*，此处只能是值传递，地址传递的话，可能函数线程还没拿到该地址的值，就被主线程更改
		assert(n==0);
	}
    close(sockfd);  //关闭连接
}
void* communication(void* arg)//函数线程完成与客户端的交互
{
	char sql[80]
  	int res;//执行sql语句后的返回标志
  	MYSQL_RES *res_ptr;//指向查询结果的指针
  	MYSQL_FIELD *field;//字段结构指针
  	MYSQL_ROW result_row;//按行返回查询信息
  	int row,column;//查询返回的行数和列数
  	MYSQL *conn;//一个数据库链接指针
  	conn = mysql_init(NULL);    //初始化连接句柄
  	if(conn == NULL) 
	{ //如果返回NULL说明初始化失败
    	printf("mysql_init failed!\n");
    	return EXIT_FAILURE;
    }
  //进行实际连接
  //参数　conn连接句柄，host　mysql所在的主机或地址,user用户名,passwd密码,database_name数据库名,后面的都是默认
  	conn = mysql_real_connect(conn,"192.168.0.105","root","123456","linuxtest",0,NULL,0);
  	if (conn) 
	{
    	printf("Connection success!\n");
  	} 
	else
	{
    	printf("Connection failed!\n");
  	}
  	mysql_query(conn,"set names gbk");//防止乱码。设置和数据库的编码一致就不会乱码
	while(1)//实现与一个客户端的多次交互
	{
	    char buff[128]={0};
	    int c=(int)(long)arg;//将文件描述符转回int型
	    int n=recv(c,buff,127,0);  //接收客户端的信息
	    if(n<=0)
	    {
			close(c);  //关闭连接
			printf("%d client over\n",c);
			break;
	    }
	    printf("%d:%s\n",c,buff);
//客户端发送的信息为一整个字符串，通过循环将不同信息分开储存，并将日期中的分隔号"."去掉		
		int i;
		char temp[10][100];
		int j = 0;
		int k = 0;
		for(i = 0; i < strlen(buff); i++)
		{
			if(buff[i] != ' ')  //遍历到空格
			{
				if(buff[i] != '.')  //遍历到"."
				{
					temp[j][k] = buff[i];
					k = k + 1;
				}	
				else
				{
					continue;
				}
			}
			else
			{
				temp[j][k] = '\0';
				j = j + 1;
				k = 0;
			}	
		}
		if(j == 5)  //j=5向数据库执行插入操作
		{	
			int temp1 = atoi(temp[1]);
			int temp2 = atoi(temp[2]);
			int temp3 = atoi(temp[3]);
			int temp4 = atoi(temp[4]);
			int temp5 = atoi(temp[5]);
			char sql[80];
			char str[80] = "insert into roomtable values('";
			strcat(str, temp[0]);  //将temp[0]的内容连接到str后
			strcat(str, "', %d, %d, %d, %d, %d)");
			printf("%s", str);
			sprintf(sql, str, temp1, temp2, temp3, temp4, temp5);
			res = mysql_query(conn, sql);
			if(res == 0)  //插入成功
			{
				send(c,"insert", 6, 0);
			}
			else  //插入失败
			{
				send(c,"error", 5, 0);
			}
		}
		if(j == 0)  //j=0向数据库执行删除操作
		{
			char sql[80];
			char str[80] = "delete from roomtable where cust_name='";
			strcat(str, temp[0]);  //将temp[0]的内容连接到str后
			strcat(str, "'");
			strcpy(sql, str);  //将str中的内容复制到sql
			printf("%s", str);
			res = mysql_query(conn, sql);
			if(res == 0)  //删除成功
			{
				send(c,"delete", 6, 0);
			}
			else  //删除失败
			{
				send(c,"error", 5, 0);
			}
		}
		if(j == 1)  //j=1向数据库执行查询操作
		{
			char res_re[10000];
			int temp1 = atoi(temp[0]);
			int temp2 = atoi(temp[1]);
			memset(res_re, 0, sizeof(res_re));
			char sql[100];
			char str[100] = "select room_num from roomtable where in_time <= %d and out_time > %d and room_type = %d";
			int g, q;
			sprintf(sql, str, temp1, temp1, temp2);
			res = mysql_query(conn, sql);
			if(res)  //查询失败
			{
				perror("my_query");
				mysql_close(conn);
				exit(0);
			}
			else  //查询成功
			{
				res_ptr = mysql_store_result(conn);
				if(res_ptr)
				{
					column = mysql_num_fields(res_ptr);
					row = mysql_num_rows(res_ptr);
					for(g = 1; g < row + 1; g++)  //对返回结果每一行进行遍历
					{
						result_row = mysql_fetch_row(res_ptr);
						for(q = 0; q < column; q++)	
						{
							strcat(res_re, result_row[q]);  //将result_row[q]的信息连接到res_re后
							strcat(res_re, " ");  //以空格隔开
						}
					}
					send(c,res_re, sizeof(res_re), 0);  //向客户端发送查询结果
				}
			}	
		}
	}
}

