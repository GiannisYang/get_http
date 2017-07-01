#include "def1.h"
#include "tools.h"

#define USE_DEFLATE 1
#define USE_GZIP 2
#define USE_ZLIB 3

pcap_t *pcap_handle;
u_char *loc_ip;
int counter;

void get_http_callback (u_char *argument,
	const pcap_pkthdr *packet_header,
	const u_char *packet_content) {
    if(packet_header->len != packet_header->caplen) return;

	int httplen = packet_header->caplen;
	const u_char *p = packet_content;
	ether_header *ethernet_head = (ether_header *) packet_content;
	if(ntohs(ethernet_head->ether_type) != 0x0800) return;

	ip_header *ip_head = (ip_header *) (packet_content + 14);
	if(!ip_head
        || ip_head->ip_protocol != 6
//        || packet_header->len - 14 != ntohs(ip_head->ip_length)
	) return;

	tcp_header *tcp_head = (tcp_header *) (packet_content + 14 +
        4 * (int)ip_head->ip_header_length);
    if(ntohs(tcp_head->tcp_source_port) != 80
        || !has_str(loc_ip, inet_ntoa(ip_head->ip_destination_address))
    ) return;

    p += 14 +  4 * ((int)ip_head->ip_header_length +
        (int)tcp_head->tcp_offset);
    httplen -= 14 +  4 * ((int)ip_head->ip_header_length +
        (int)tcp_head->tcp_offset);

    if(!has_str(p, "HTTP/1.1 200")) return;

/////////////////////// tiao shi
cout <<endl<<"======================="<< httplen<< endl;
    cout<<"From: "<<inet_ntoa(ip_head->ip_souce_address)
    <<": "<<ntohs(tcp_head->tcp_source_port)<<" to loc: "
        <<ntohs(tcp_head->tcp_destination_port)<< endl;

    cout << "tcp_acknowledgement: "<<
        ntohs(tcp_head->tcp_acknowledgement)<<endl;


    /** Process The Http Header */
    int content_len = 0, encoding = 0;
    bool use_chunked = false;
    for(int i = 0; i < httplen; i++) {
        if(*p == '\r' && has_str(p, "\r\n\r\n")) {
            p += 4;
            break;
        }

    /** Content-Length = 0: maybe response packet  */
        if(*p == 'C' && has_str(p, "Content-Length")) {
//            p += strlen("Content-Length: ");
            content_len = get_num((p + 16), '\r');
        }

    /** If use chunked */
        if(*p == 'T' && has_str(p, "Transfer-Encoding: chunked")) {
//            p += strlen("Transfer-Encoding: chunked\r\n");
            use_chunked = true;
        }

        if(*p == 'C' && has_str(p, "Content-Encoding")) {
//            p += strlen("Content-Encoding: ");
            if(has_str((p + 18), "deflate")) encoding = USE_DEFLATE;
            if(has_str((p + 18), "gzip")) encoding = USE_GZIP;
        }

        cout <<*p++;
    }

    cout <<endl;
    /** Process The Http Content */
    if(use_chunked) {
        int n;
        while(n = get_num(p, '\r')) {
            while(*p != '\r') p++;
            p += 2;
            while(*p != '\r')
                cout << *p++;
            p += 2;
        }
    } else if(!content_len) {
        return;
    } else {
        for(int i = 0; i < content_len; i++) {
            cout << *p++;
        }
    }






    cout <<endl<<"================================="<< endl;
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

