#include "def1.h"
#include "tools.h"

pcap_t *pcap_handle;
u_char *loc_ip;
int counter;

void get_http_callback (u_char *argument,
	const pcap_pkthdr *packet_header,
	const u_char *packet_content) {

	const u_char *p = packet_content;
	ether_header *ethernet_head = (ether_header *) packet_content;
	ip_header *ip_head = (ip_header *) (packet_content + 14);
	tcp_header *tcp_head = (tcp_header *) (packet_content + 34);
	int httplen = packet_header->caplen - 34;

	p += 34;
    if(packet_header->len != packet_header->caplen
        || !ip_head
        || !httplen
        || ntohs(ethernet_head->ether_type) != 0x0800
        || ip_head->ip_protocol != 6
        || ntohs(tcp_head->tcp_source_port) != 80
        || !has_str(loc_ip, inet_ntoa(ip_head->ip_destination_address))
        || !has_str(p, "HTTP/1.1 200")
    ) { return; }


    for(int i=0;i<httplen;i++)
        cout <<*p++;

    counter++;
    if(counter>=10)
        pcap_breakloop(pcap_handle);

}


int main() {

    counter = 0;
	char error_content[PCAP_ERRBUF_SIZE]; // err info
	char *net_interface; // network interface
	struct bpf_program	bpf_filter; /* BPF过滤规则 */
	char bpf_filter_string[] = "tcp";
	/* 过滤规则字符串，此时表示本程序只捕获TCP协议的网络数据包 */
	bpf_u_int32 net_mask;
	bpf_u_int32 net_ip;
	net_interface = pcap_lookupdev(error_content);
	/* 获得网络接口 */

	loc_ip = get_loc_ip();
	cout << loc_ip << endl;

	pcap_lookupnet(net_interface, &net_ip, &net_mask, error_content);
	/* 获得网络地址和网络掩码 */
	pcap_handle = pcap_open_live(net_interface, BUFSIZ, 1, 0, error_content);
	/* 打开网络设备 */
	pcap_compile(pcap_handle, &bpf_filter, bpf_filter_string, 0, net_ip);
	/* 编译过滤规则 */
	pcap_setfilter(pcap_handle, &bpf_filter);
	/* 设置过滤规则 */
	if(pcap_datalink(pcap_handle) != DLT_EN10MB) return 0;
	pcap_loop(pcap_handle, -1, get_http_callback, NULL);
	pcap_close(pcap_handle);

	return 0;
}

