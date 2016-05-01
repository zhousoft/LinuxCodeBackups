#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<net/if.h>
#include<string.h>

int main()
{
	int sockfd;
	struct ifreq ifr;
	struct sockaddr_in sin;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		return -1;
	}
	strcpy(ifr.ifr_name,"wlan0");
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)//直接获取IP地址
	{
		perror("ioctl error");
		return -1;
	}
	memcpy(&sin, &ifr.ifr_dstaddr, sizeof(sin));
	printf("ip is %s \n",inet_ntoa(sin.sin_addr));
	return 0;
}
