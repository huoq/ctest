#include <linux/netlink.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main()
{
	int fd = socket(AF_NETLINK,SOCK_RAW,NETLINK_TEST);
	printf("fd=%d\n",fd);

	struct sockaddr_nl sa, da;
	//src, current process
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_pid = getpid();
	//dst, kernel 
	memset(&da, 0, sizeof(da));
	da.nl_family = AF_NETLINK;
	da.nl_pid = 0;
	bind(fd, (struct sockaddr*)&sa, sizeof(sa));

	struct msghdr msg;   
	memset(&msg, 0, sizeof(msg));   
	msg.msg_name = (void *)&(da);   
	msg.msg_namelen = sizeof(da);  

	//send
	char buffer[] = "Hello from userspace!";   
	struct nlmsghdr *nlhdr;   
	nlhdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(1024));   
	strcpy(NLMSG_DATA(nlhdr),buffer);   
	nlhdr->nlmsg_len = NLMSG_LENGTH(strlen(buffer));   
	nlhdr->nlmsg_pid = getpid();   
	nlhdr->nlmsg_flags = 0;

	struct iovec iov;   
	iov.iov_base = (void *)nlhdr;   
	iov.iov_len = nlhdr->nlmsg_len;   
	msg.msg_iov = &iov;   
	msg.msg_iovlen = 1;  

	sendmsg(fd, &msg, 0);

	//receive
	printf("Waiting message from kernel ...\n");
	memset(nlhdr, 0, NLMSG_SPACE(1024));
	recvmsg(fd, &msg, 0);
	//strcpy(buffer, NLMSG_DATA(nlhdr));   
	printf("Received: %s\n", NLMSG_DATA(nlhdr));

	close(fd);
}

