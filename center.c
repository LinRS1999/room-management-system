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
#include<netdb.h>
struct node         //定义节点结构体
{
	int c;
	int a;
};
void* communication(void* arg);
int main()
{
	int sockfdto = socket(PF_INET,SOCK_STREAM,0);     //创建套接字
	assert(-1!=sockfdto);    //创建失败返回错误
	struct sockaddr_in serto,clito;     //套接字地址
	memset(&serto,0,sizeof(serto));
	serto.sin_family=AF_INET;  //定义地址域为IPv4
	serto.sin_port=htons(3000);    //设置端口号
	struct hostent *host;	  //定义host
	host = gethostbyname("192.168.0.105");     //输入服务器IPv4地址
	serto.sin_addr=*((struct in_addr *)host -> h_addr);   设置IPv4地址 
	int resto=connect(sockfdto,(struct sockaddr*)&serto,sizeof(serto));    //发起连接
	assert(resto!=-1);   //连接失败返回错误
	int sockfd=socket(PF_INET,SOCK_STREAM,0);   //创建套接字
	assert(-1!=sockfd);    //创建失败返回错误
	struct sockaddr_in ser,cli;   //套接字地址
	ser.sin_family=AF_INET;   //定义地址域为IPv4
	ser.sin_port=htons(6000);   //设置端口号
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
		struct node *b;  //定义节点b
		b = (struct node *)malloc(sizeof(struct node));
		b->c = c;   //指针b->c赋值
		b->a = sockfdto;  //指针b->a赋值
		int n=pthread_create(&id,NULL,communication,(void*)b);//创建线程，将文件描述符强转为void*，此处只能是值传递，地址传递的话，可能函数线程还没拿到该地址的值，就被主线程更改
		assert(n==0);
	}
    close(sockfd);  //关闭连接
}
void* communication(void* arg)//函数线程完成与客户端的交互
{
	struct node *temp;  //定义节点temp
	temp = (struct node*)arg;
	while(1)//实现与一个客户端的多次交互
	{
	    char buff[1000]={0};
	    int c=temp->c;//将文件描述符转回int型
	    int n=recv(c,buff,127,0);   //接收客户端的信息
	    if(n<=0)
	    {
			close(c);  //关闭连接
			printf("%d client over\n",c);
			break;
	    }
		send(temp->a, buff, strlen(buff), 0);   //发送信息到服务器
	    printf("%d:%s\n",c,buff);
	    char buffto[1000]={0};
	    int nto = recv(temp->a, buffto, 127, 0);   //接受服务器信息
	    printf("%d:%s\n", temp->a, buffto);
		send(c, buffto, strlen(buffto), 0);  //发送信息到客户端
	}
}

