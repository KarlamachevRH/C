#include "get_dest_mac.h"
#include "headers.h"
#include <net/ethernet.h> /* the L2 protocols */
#include <linux/if_link.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>

int raw_sockfd; // дескриптор сокета

/* Печать адреса клиента */
void print_server_data(struct sockaddr_in serv_addr)
{
	char server_addr[ADDR_LEN];
	printf("Server address: %s:%d\n", \
			inet_ntop(AF_INET, &serv_addr.sin_addr.s_addr, server_addr, ADDR_LEN), \
			ntohs(serv_addr.sin_port));
}

static void sig_handler(int sig)
{
	close(raw_sockfd);
#ifdef DEBUG
	printf("\n");
#endif //DEBUG
	log_info("End of client program");
	exit(EXIT_SUCCESS);
}

in_addr_t *get_interface_ip(char **argv, in_addr_t *ip)
{
	struct ifaddrs *ifaddr, *ifa;
	
	if (getifaddrs(&ifaddr) == -1) 
		handle_error("getifaddrs");

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;
		if (strncmp(argv[3], ifa->ifa_name, strlen(argv[3])+1) == 0)
			memmove(ip, ifa->ifa_addr->sa_data, sizeof(*ip));
	}
	freeifaddrs(ifaddr);
	return ip;
}

uint16_t calculate_checksum(struct iphdr *ip_header)
{
	int csum_tmp = 0;
	short *csum_tmp_ptr;
	uint16_t csum = 0;
	short *ptr;
	char buf[MAX_MSG_BUF_SIZE/2];
	memset(buf, 0, MAX_MSG_BUF_SIZE/2);
	memmove(buf, ip_header, sizeof(struct iphdr));
	ptr = (short*)buf;
	while(*ptr)
	{
		csum_tmp = csum_tmp + *ptr;
		ptr++;
	}
	csum_tmp_ptr = (short*)&csum_tmp;
	if(*csum_tmp_ptr != 0)
	{
		csum = csum + *csum_tmp_ptr;
		csum_tmp_ptr++;
		csum = csum + *csum_tmp_ptr;
	}
	else	
		csum = csum_tmp;
	return csum;
}

/* Заполнение заголовка протокола IP для RAW - сокета */
void set_ip_header(struct iphdr *ip_header, size_t size, \
				   in_addr_t interface_ip, in_addr_t dest_addr)
{
	uint16_t csum = 0;
	ip_header->version = IPVERSION;
	ip_header->ihl = 5;
	ip_header->tos = 0;
	ip_header->tot_len = htons(size);
	ip_header->id = 0;
	ip_header->frag_off = 0;
	ip_header->ttl = IPDEFTTL;
	ip_header->protocol = IPPROTO_UDP;
	ip_header->check = 0;
	ip_header->saddr = interface_ip;
	ip_header->daddr = dest_addr;
	csum = calculate_checksum(ip_header);
	ip_header->check = csum;
}

void get_hwadrr(int sockfd, unsigned char *mac_address)
{
	struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) == -1) 
		handle_error("ioctl");

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end && success == 0; ++it)
	{
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0)
		{
            if ((ifr.ifr_flags & IFF_LOOPBACK) == 0) 
			{ // don't count loopback
                if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0)				
                    success = 1;
            }
        }
        else 
			handle_error("ioctl");
    }
    if (success)
		memcpy(mac_address, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
}

int main(int argc, char **argv) 
{
	log_info("Start of client program");

	int port_num; /* Номер порта */
	int bytes_sended = 0;
	int bytes_recieved = 0;
	in_addr_t interface_ip;
	in_addr_t dest_addr;
	struct sockaddr_in recieved_addr;
	struct sockaddr_ll server_ll_addr;
	struct hostent *server;
	unsigned int if_index = 0;
	char sending_buf[MAX_MSG_BUF_SIZE] = {0};
	char recieving_buf[MAX_MSG_BUF_SIZE] = {0};
	char *msg = "Hello from client!";

	struct udphdr udp_header; /* Заголовок протокола UDP для RAW - сокета */
	struct iphdr ip_header; /* Заголовок протокола IP для RAW - сокета */
	struct ether_header mac_header; /* Заголовок протокола Ethernet для RAW - сокета */
	memset(&mac_header, 0, sizeof(mac_header));

	if(argc != 4 || strcmp(argv[1], "--help") == 0)
	{
		printf("%s <interface name> <server address> <port number (range: 1025 - 65535)>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Создание сокета */
	raw_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (raw_sockfd < 0)
		handle_error("error opening socket");
	
	if ((signal(SIGINT, sig_handler)) == SIG_ERR)
	{
		perror("exit");
		exit(EXIT_FAILURE);
	}

	if (strlen(argv[1]) > (IFNAMSIZ - 1)) 
	{
        printf("Too long interface name, MAX=%i\n", IFNAMSIZ - 1);
        exit(EXIT_FAILURE);
    }

	/* Индекс интерфейса */
	if((if_index = if_nametoindex(argv[1])) == 0)
		handle_error("if_nametoindex");
		
	/* Извлечение хоста */
	server = gethostbyname(argv[2]);
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(EXIT_FAILURE);
	}

	/* Извлечение порта */
	port_num = atoi(argv[3]);
	if(port_num != 0 && port_num < 1025 && port_num > 65535)
	{
		printf("Port number is incorrect\n");
		exit(EXIT_FAILURE);
	}
	
	get_interface_ip(argv, &interface_ip);

	memset(&server_ll_addr, 0, sizeof(server_ll_addr));
	server_ll_addr.sll_family = AF_PACKET;
	server_ll_addr.sll_ifindex = if_index;
	server_ll_addr.sll_halen = ETHER_ADDR_LEN;

	/* Размер датаграммы на транспортном, затем на сетевом уровне*/
	size_t size = 0;
	size_t payload_size = strlen(msg) + 1;

	/* Заполнение заголовка протокола UDP для RAW - сокета */
	udp_header.source = htons(port_num - 1);
	udp_header.dest = htons(port_num);
	size = payload_size + sizeof(udp_header);
	udp_header.len = htons(size);
	udp_header.check = 0;

	/* Заполнение заголовка протокола IP для RAW - сокета */
	size = size + sizeof(ip_header);
	memmove(&dest_addr, server->h_addr_list[0], server->h_length);
	set_ip_header(&ip_header, size, interface_ip, dest_addr);

	unsigned char dest_mac[ETHER_ADDR_LEN] = {0};
	size = size + sizeof(mac_header);
	/* Destination MAC - address */
	get_dest_mac(argv[1], &dest_addr, dest_mac);
	log_info("Destination MAC: %02X:%02X:%02X:%02X:%02X:%02X",
			dest_mac[0],
			dest_mac[1],
			dest_mac[2],
			dest_mac[3],
			dest_mac[4],
			dest_mac[5]);
	memmove(mac_header.ether_dhost, dest_mac, ETHER_ADDR_LEN);
	/* Source interface MAC - address */
	get_hwadrr(raw_sockfd, mac_header.ether_shost);
	mac_header.ether_type = htons(ETHERTYPE_IP);

	/* Вызываем bind для связывания */
	/* if (bind(raw_sockfd, (struct sockaddr *)&server_ll_addr, sizeof(server_ll_addr)) < 0) 
		handle_error("error on binding"); */

	char *p = sending_buf;
	memmove(p, &mac_header, sizeof(mac_header));
	p = p + sizeof(mac_header);
	memmove(p, &ip_header, sizeof(ip_header));
	p = p + sizeof(ip_header);
	memmove(p, &udp_header, sizeof(udp_header));
	p = p + sizeof(udp_header);
	unsigned int maxlen = MAX_MSG_BUF_SIZE - sizeof(ip_header) - \
						  sizeof(udp_header) - sizeof(mac_header);
	snprintf(p, maxlen, "%s", msg);

	/* Отправка данных на сервер */
	bytes_sended = sendto(raw_sockfd, sending_buf, size, 0, \
						 (struct sockaddr *)&server_ll_addr, sizeof(server_ll_addr));
	if (bytes_sended < 0) 
		handle_error("ERROR send data to socket");
	log_info("bytes_sended: %d", bytes_sended);
	printf("Sended string to server: %s\n", p);
	
	socklen_t len = sizeof(struct sockaddr_in);

	/* 
	 * Сместим указатель в приемном буфере до адреса переменной, 
	 * в которой хранится порт назначения 
	 */
	uint8_t *rbp = (uint8_t*)&recieving_buf[0];
	size = sizeof(mac_header) + sizeof(struct iphdr) + sizeof(udp_header.source);
	rbp = rbp + size;

	while (1)
	{
		bytes_recieved = recvfrom(raw_sockfd, recieving_buf, MAX_MSG_BUF_SIZE, 0, \
								 (struct sockaddr *)&recieved_addr, &len);
		if (bytes_recieved < 0)
			handle_error("ERROR reading from socket");
		else if(bytes_recieved == 0)
				printf("No data recieved from server\n");
		log_info("bytes_recieved: %d", bytes_recieved);
		print_server_data(recieved_addr);

		/* 
		 * Порт назначения из UDP - заголовка в приемном буфере должен
		 * совпадать с портом отправителя, заполненном нами 
		 */
		log_info("recieved datagram destination port: %d", ntohs(*(uint16_t*)rbp));
		log_info("sended datagram source port: %d", ntohs(udp_header.source));
		if(*(uint16_t*)rbp == udp_header.source) 
		{
			rbp = (uint8_t*)rbp + sizeof(struct udphdr) - sizeof(udp_header.source);
			printf("Message from server: %s\n", (char*)rbp);
			break;
		}
	}

	close(raw_sockfd);
	log_info("End of client program");
	exit(EXIT_SUCCESS);
}
