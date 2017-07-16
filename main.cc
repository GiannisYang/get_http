#ifndef MAIN_CC
#define MAIN_CC

#include "def1.h"
#include "tools.h"
#include "html_content.h"
#include "min_heap.h"

#define USE_ZLIB 3
#define BUF_SIZE 20000
#define ACK 0x10
#define RST 0x04
#define SYN 0x02
#define FIN 0x01

extern pthread_mutex_t list_lock;
u_char *loc_ip;

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
        || !has_str(loc_ip, inet_ntoa(ip_head->ip_destination_address))
//        || packet_header->caplen - 14 != ntohs(ip_head->ip_length)
	) return;

//	if(ip_head->ip_off > 0)
//        cout << "ip_off: " << ip_head->ip_off << endl;

	tcp_header *tcp_head = (tcp_header *) (packet_content + 14 +
        4 * (int)ip_head->ip_header_length);
    if(!tcp_head
        || ntohs(tcp_head->tcp_source_port) != 80
        /** SYN is used to create a connection without data */
        || tcp_head->tcp_flags & SYN
        /** ACK must be 1 */
        || !(tcp_head->tcp_flags & ACK)
    ) return;

    httplen -= 14 +  4 * ((int)ip_head->ip_header_length +
        (int)tcp_head->tcp_offset);
    p += 14 +  4 * ((int)ip_head->ip_header_length +
        (int)tcp_head->tcp_offset);

    /** If the http response head is to small, it might
      * be a very short (6 bytes) packet with blank lines */
    if(httplen <= 6
//        || (has_str(p, "HTTP") && !has_str(p + 9, "200"))
    )   return;

//    cout <<"t1: "<< <<endl;
    /** locate the html in globe html list */
    html_content* hp;

    if((hp = find_html(ntohl(tcp_head->tcp_ack))) == NULL) {
        if(!has_str(p, "HTTP/1.1 200 OK"))
            return;
        hp = insert_html(ntohl(tcp_head->tcp_ack));
    }

    /** add content into html */
    hp->add(p, httplen);
    hp->p_html();

return;

    cout <<"--------------"<<httplen<<"--------------"<<endl;
    cout << "tcp_acknowledgement: "<<
        ntohl(tcp_head->tcp_ack)<<endl;
    cout << "tcp_sequence: "<<
        ntohl(tcp_head->tcp_sequence)<<endl;
    cout <<"--------------------------------------"<<endl;

}

void* process_http(void *arg) {
    int res;
    html_content* hp;

    while(1) {
        hp = find_html();
        if(!hp) {
            sleep(1);
            continue;
        }

    /** Process The Http Header */
        res = hp->process_head();

    /** res = -1: the head is not fully received yet..
        * we shall wait */
        if(res == -1) continue;

    /** res = -2: Content-Length = 0
        * the packet has to be discard */
        if(res == -2) {
            hp = delete_html(hp->tcp_ack);
            delete hp;
        } else {
            hp->write2file();
    /** res = 0: the head is fully received */
            if(hp->process_tail() == 0) {
                hp = delete_html(hp->tcp_ack);
                while(hp->write2file() != 0);
                delete hp;
            }
        }
    }
}


int main() {

//char data[] = "kjdalkfjdflkjdlkfjdkdfaskf;ldsfk;ldakf;ldskfl;dskf;ld";
//uLong ndata = strlen(data);
//Bytef zdata[BUF_SIZE];
//uLong nzdata = BUF_SIZE;
//Bytef odata[BUF_SIZE];
//uLong nodata = BUF_SIZE;
//memset(zdata, 0, BUF_SIZE);
////if(zcompress((Bytef *)data, ndata, zdata, &nzdata) == 0)
//if(gz_compress((Bytef *)data, ndata, zdata, &nzdata) == 0)
//{
//fprintf(stdout, "nzdata:%d %s\n", nzdata, zdata);
//memset(odata, 0, BUF_SIZE);
////if(zdecompress(zdata, ndata, odata, &nodata) == 0)
//if(gz_decompress(zdata, ndata, odata, &nodata) == 0)
//{
//fprintf(stdout, "%d %s\n", nodata, odata);
//}
//}

    loc_ip = get_loc_ip();
	cout << loc_ip << endl;
    pthread_mutex_init(&list_lock, NULL);

    pthread_t tid;
    if(pthread_create(&tid, NULL, process_http, NULL) != 0) {
        cout << "pthread_create() != 0" << endl;
        return 0;
    }

    pcap_t *pcap_handle;
	char error_content[PCAP_ERRBUF_SIZE]; // err info
	char *net_interface; // network interface
	struct bpf_program	bpf_filter; /* BPF过滤规则 */
	char bpf_filter_string[] = "tcp";
	/* 过滤规则字符串，此时表示本程序只捕获TCP协议的网络数据包 */
	bpf_u_int32 net_mask;
	bpf_u_int32 net_ip;
	net_interface = pcap_lookupdev(error_content);
	/* 获得网络接口 */
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
    pthread_join(tid, NULL);

	return 0;
}

#endif
