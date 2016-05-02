#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "processpool.h"//进程池头文件

//处理客户CGI请求的类，作为processpool类的模板参数
class cgi_conn
{
public:
	cgi_conn(){}
	~cgi_conn(){}
	//初始化客户端连接并清空缓冲区
	void init(int epollfd, int sockfd, const sockaddr_in& client_addr)
	{
		m_epollfd = epollfd;
		m_sockfd = sockfd;
		m_address = client_addr;
		memset(m_buf, '\0', BUFFER_SIZE);
		m_read_idx = 0;
	}
	//具体的处理逻辑任务函数
	void process()
	{
		int idx = 0;
		int ret = -1;
		//循环读取并分析客户数据
		while(true)
		{
			idx = m_read_idx;
			ret = recv(m_sockfd, m_buf + idx, BUFFER_SIZE - idx -1, 0);
			//如果读操作发生错误，关闭客户端连接，但暂时无数据可读时，退出循环
			if(ret < 0)
			{
				if(errno != EAGAIN)
				{
					removefd(m_epollfd, m_sockfd);
				}
				break;
			}
			//如果对方关闭连接，服务器也关闭连接
			else if(ret == 0)
			{
				removefd(m_epollfd, m_sockfd);
			}
			else
			{
				m_read_idx += ret;
				printf("user content is %s\n",m_buf);
				//如果遇到字符"\r\n"，则开始处理客户端请求
				for(;idx < m_read_idx; idx++)
				{
					if((idx >= 1) && (m_buf[idx-1] == '\r') && (m_buf[idx] == '\n'))
					{
						break;
					}
				}
				//没有遇到\r\n,需要读取更多字符
				if(idx == m_read_idx)
				{
					continue;
				}
				m_buf[idx-1] = '\0';

				char *file_name = m_buf;
				//判断客户要运行的CGI程序是否存在
				if(access(file_name, F_OK) == -1)
				{
					removefd(m_epollfd, m_sockfd);
					break;
				}
				//创建子进程来运行CGI程序
				ret = fork();
				if(ret == -1)
				{
					removefd(m_epollfd, m_sockfd);
					break;
				}
				else if(ret > 0)
				{
					//父进程只需关闭连接
					removefd(m_epollfd, m_sockfd);
					break;
				}
				else
				{
					//子进程将标准输出定向到m_sockfd,并执行CGI程序
					close(STDOUT_FILENO);
					dup(m_sockfd);
					execl(m_buf, m_buf, NULL);
					exit(0);
				}

			}
			
		}
	}

private:
	static const int BUFFER_SIZE = 1024;//读缓冲区大小
	static int m_epollfd;
	int m_sockfd;
	sockaddr_in m_address;
	char m_buf[BUFFER_SIZE];
	int m_read_idx;//读缓冲中已经读入的客户数据字节数（标记已读入数据的最后一个字节的下一个位置）
	
};

int cgi_conn::m_epollfd = -1;

int main(int argc, char* argv[])
{
	if(argc <= 2)
	{
		printf("usage: %s ip_address port_number\n",basename(argv[0]));
	}

	const char *ip = argv[1];
	int port = atoi(argv[2]);

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	int ret = 0;
	struct sockaddr_in address;
	bzero(&address,sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET,ip, &address.sin_addr);
	address.sin_port = htons(port);

	ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(listenfd,5);
	assert(ret != -1);
	//创建进程池
	processpool<cgi_conn> * pool = processpool<cgi_conn>::create(listenfd);
	if(pool)
	{
		pool->run();
		delete pool;
	}
	close(listenfd);//由创建者关闭listenfd
	return 0;

}

