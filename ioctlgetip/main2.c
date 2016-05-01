#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <string.h>

int main()
{
	int i = 0;
	int sockfd;
	struct ifconf ifconf;
	struct ifreq *ifreq;
	unsigned char buf[1024];

	//初始化ifconf
	ifconf.ifc_len = 1024;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(1);
	}

	//获取所有接口信息
	ioctl(sockfd, SIOCGIFCONF, &ifconf);

	//逐个获取Ip地址
	ifreq = (struct ifreq*)buf;
	for(i = (ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
		printf("name = [%s] : ",ifreq->ifr_name);
		printf("%s\n",inet_ntoa( ((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr));
		ifreq++;
	}
	return 0;
}

