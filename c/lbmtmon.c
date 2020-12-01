/*
  lbmtmon.c: example LBM monitoring application, including "tmon".
  Cloned from lbmmon.c

  Copyright (c) 2020 Informatica Corporation  Permission is granted to licensees to use
  or alter this software for any purpose, including commercial applications,
  according to the terms laid out in the Software License Agreement.

  This source code example is provided by Informatica for educational
  and evaluation purposes only.

  THE SOFTWARE IS PROVIDED "AS IS" AND INFORMATICA DISCLAIMS ALL WARRANTIES
  EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF
  NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR
  PURPOSE.  INFORMATICA DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE
  UNINTERRUPTED OR ERROR-FREE.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES, BE
  LIABLE TO LICENSEE FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR
  INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGREEMENT OR THE
  TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BEEN APPRISED OF
  THE LIKELIHOOD OF SUCH DAMAGES.
*/

#ifdef __VOS__
#define _POSIX_C_SOURCE 200112L
#include <sys/time.h>
#endif
#if defined(__TANDEM) && defined(HAVE_TANDEM_SPT)
	#include <ktdmtyp.h>
	#include <spthread.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
	#pragma warning(disable : 4996)
	#include <winsock2.h>
	#define strcasecmp stricmp
#else
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <signal.h>
	#if defined(__TANDEM)
		#include <strings.h>
	#endif
#endif
#include "replgetopt.h"
#include <lbm/lbm.h>
#include <lbm/lbmmon.h>
#include "tmon.h"
#include "monmodopts.h"

/* The following macros are used with sscanf.
 * See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
#define STR2(_s) #_s
#define STR(_s) STR2(_s)

#if defined(_WIN32)
#   define SLEEP_SEC(x) Sleep((x)*1000)
#   define SLEEP_MSEC(x) Sleep(x)
#else
#   define SLEEP_SEC(x) sleep(x)
#   define SLEEP_MSEC(x) \
		do{ \
			if ((x) >= 1000){ \
				sleep((x) / 1000); \
				usleep((x) % 1000 * 1000); \
			} \
			else{ \
				usleep((x)*1000); \
			} \
		}while (0)
#endif /* _WIN32 */

#define ASSRT(cond_) do { \
	if (! (cond_)) { \
		fprintf(stderr, "Failed assert at %s:%d '%s'\n", \
			__FILE__, __LINE__, #cond_); \
		fflush(stderr); \
		exit(1); \
	} \
} while (0)

const char Purpose[] = "Purpose: Example LBM statistics monitoring application.";
const char Usage[] =
"Usage: %s [options]\n"
"Available options:\n"
"  -c, --config=FILE          Use LBM configuration file FILE.\n"
"                             Multiple config files are allowed.\n"
"                             Example:  '-c file1.cfg -c file2.cfg'\n"
"  -h, --help                 display this help and exit\n"
"  -t, --transport=TRANS      use transport module TRANS\n"
"                             TRANS may be `lbm', `udp', or `lbmsnmp', default is `lbm'\n"
"      --transport-opts=OPTS  use OPTS as transport module options\n"
"  -f, --format=FMT           use format module FMT\n"
"                             FMT may be `csv'\n"
"      --format-opts=OPTS     use OPTS as format module options\n"
MONMODULEOPTS_RECEIVER;

const char * OptionString = "c:f:ht:";
const struct option OptionTable[] =
{
	{ "config", required_argument, NULL, 'c' },
	{ "help", no_argument, NULL, 'h' },
	{ "transport", required_argument, NULL, 't' },
	{ "transport-opts", required_argument, NULL, 0 },
	{ "format", required_argument, NULL, 'f' },
	{ "format-opts", required_argument, NULL, 1 },
	{ NULL, 0, NULL, 0 }
};
int log_callback(int Level, const char * Message, void * ClientData);

void dump_hex_data(const unsigned char * Data, size_t Length)
{
	static char hextable[16] =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	unsigned char c;
	size_t idx = 0;

	while (idx < Length)
	{
		c = Data[idx];
		printf("%c", hextable[((c >> 4) & 0x0f)]);
		printf("%c", hextable[(c & 0x0f)]);
		idx++;
	}
}

void print_attributes(const char * Message, const void * AttributeBlock)
{
	struct in_addr addr;
	char appid[256];
	time_t timestamp;
	char * time_string;
	lbm_ulong_t objectid = 0;
	lbm_ulong_t processid = 0;
	lbm_uint8_t ctxinst[LBM_CONTEXT_INSTANCE_BLOCK_SZ];
	lbm_uint32_t domain_id;

	printf("\n%s", Message);
	if (lbmmon_attr_get_appsourceid(AttributeBlock, appid, sizeof(appid)) == 0)
	{
		printf(" from %s", appid);
	}
	if (lbmmon_attr_get_ipv4sender(AttributeBlock, (lbm_uint_t *) &(addr.s_addr)) == 0)
	{
		printf(" at %s", inet_ntoa(addr));
	}
	if (lbmmon_attr_get_processid(AttributeBlock, &(processid)) == 0)
	{
		printf(", process ID=%0lx", processid);
	}
	if (lbmmon_attr_get_objectid(AttributeBlock, &(objectid)) == 0)
	{
		printf(", object ID=%0lx", objectid);
	}
	if (lbmmon_attr_get_ctxinst(AttributeBlock, ctxinst, sizeof(ctxinst)) == 0)
	{
		printf(", context instance=");
		dump_hex_data((unsigned char *) ctxinst, sizeof(ctxinst));
	}
	if (lbmmon_attr_get_domainid(AttributeBlock, &domain_id) == 0)
	{
		printf(", domain ID=%u", domain_id);
	}
	if (lbmmon_attr_get_timestamp(AttributeBlock, &timestamp) == 0)
	{
		time_string = ctime(&timestamp);
		printf(", sent %s", time_string);
		/* Reminder: ctime() returns a string terminated with a newline */
	}
	else
	{
		printf("\n");
	}
}

const char * translate_transport(int Type)
{
	switch (Type)
	{
		case LBM_TRANSPORT_STAT_TCP:
			return ("TCP");

		case LBM_TRANSPORT_STAT_LBTRM:
			return ("LBT-RM");

		case LBM_TRANSPORT_STAT_LBTRU:
			return ("LBT-RU");

		case LBM_TRANSPORT_STAT_LBTIPC:
			return ("LBT-IPC");

		case LBM_TRANSPORT_STAT_LBTRDMA:
			return ("LBT-RDMA");

		default:
			return ("Unknown");
	}
}

const char * translate_pattern_type(lbm_uint8_t Type)
{
	switch (Type)
	{
		case LBM_WILDCARD_RCV_PATTERN_TYPE_PCRE:
			return ("PCRE");
		case LBM_WILDCARD_RCV_PATTERN_TYPE_REGEX:
			return ("Regex");
		default:
			return ("Unknown");
	}
}

void rcv_statistics_cb(const void * AttributeBlock, const lbm_rcv_transport_stats_t * Statistics, void * ClientData)
{
	lbm_ulong_t source = LBMMON_ATTR_SOURCE_NORMAL;

	if (lbmmon_attr_get_source(AttributeBlock, &source) != 0)
	{
		source = LBMMON_ATTR_SOURCE_NORMAL;
	}
	switch (source)
	{
		case LBMMON_ATTR_SOURCE_IM:
			print_attributes("Context IM receiver statistics received", AttributeBlock);
			break;
		default:
			print_attributes("Receiver statistics received", AttributeBlock);
			break;
	}
	printf("Source: %s\n", Statistics->source);
	printf("Transport: %s\n", translate_transport(Statistics->type));
	switch (Statistics->type)
	{
		case LBM_TRANSPORT_STAT_TCP:
			printf("\tLBT-TCP bytes received                                    : %lu\n", Statistics->transport.tcp.bytes_rcved);
			printf("\tLBM messages received                                     : %lu\n", Statistics->transport.tcp.lbm_msgs_rcved);
			printf("\tLBM messages received with uninteresting topic            : %lu\n", Statistics->transport.tcp.lbm_msgs_no_topic_rcved);
			printf("\tLBM requests received                                     : %lu\n", Statistics->transport.tcp.lbm_reqs_rcved);
			break;

		case LBM_TRANSPORT_STAT_LBTRM:
			printf("\tLBT-RM datagrams received                                 : %lu\n", Statistics->transport.lbtrm.msgs_rcved);
			printf("\tLBT-RM datagram bytes received                            : %lu\n", Statistics->transport.lbtrm.bytes_rcved);
			printf("\tLBT-RM NAK packets sent                                   : %lu\n", Statistics->transport.lbtrm.nak_pckts_sent);
			printf("\tLBT-RM NAKs sent                                          : %lu\n", Statistics->transport.lbtrm.naks_sent);
			printf("\tLost LBT-RM datagrams detected                            : %lu\n", Statistics->transport.lbtrm.lost);
			printf("\tNCFs received (ignored)                                   : %lu\n", Statistics->transport.lbtrm.ncfs_ignored);
			printf("\tNCFs received (shed)                                      : %lu\n", Statistics->transport.lbtrm.ncfs_shed);
			printf("\tNCFs received (retransmit delay)                          : %lu\n", Statistics->transport.lbtrm.ncfs_rx_delay);
			printf("\tNCFs received (unknown)                                   : %lu\n", Statistics->transport.lbtrm.ncfs_unknown);
			printf("\tLoss recovery minimum time                                : %lums\n", Statistics->transport.lbtrm.nak_stm_min);
			printf("\tLoss recovery mean time                                   : %lums\n", Statistics->transport.lbtrm.nak_stm_mean);
			printf("\tLoss recovery maximum time                                : %lums\n", Statistics->transport.lbtrm.nak_stm_max);
			printf("\tMinimum transmissions per individual NAK                  : %lu\n", Statistics->transport.lbtrm.nak_tx_min);
			printf("\tMean transmissions per individual NAK                     : %lu\n", Statistics->transport.lbtrm.nak_tx_mean);
			printf("\tMaximum transmissions per individual NAK                  : %lu\n", Statistics->transport.lbtrm.nak_tx_max);
			printf("\tDuplicate LBT-RM datagrams received                       : %lu\n", Statistics->transport.lbtrm.duplicate_data);
			printf("\tLBT-RM datagrams unrecoverable (window advance)           : %lu\n", Statistics->transport.lbtrm.unrecovered_txw);
			printf("\tLBT-RM datagrams unrecoverable (NAK generation expiration): %lu\n", Statistics->transport.lbtrm.unrecovered_tmo);
			printf("\tLBT-RM LBM messages received                              : %lu\n", Statistics->transport.lbtrm.lbm_msgs_rcved);
			printf("\tLBT-RM LBM messages received with uninteresting topic     : %lu\n", Statistics->transport.lbtrm.lbm_msgs_no_topic_rcved);
			printf("\tLBT-RM LBM requests received                              : %lu\n", Statistics->transport.lbtrm.lbm_reqs_rcved);
			printf("\tLBT-RM datagrams dropped (size)                           : %lu\n", Statistics->transport.lbtrm.dgrams_dropped_size);
			printf("\tLBT-RM datagrams dropped (type)                           : %lu\n", Statistics->transport.lbtrm.dgrams_dropped_type);
			printf("\tLBT-RM datagrams dropped (version)                        : %lu\n", Statistics->transport.lbtrm.dgrams_dropped_version);
			printf("\tLBT-RM datagrams dropped (hdr)                            : %lu\n", Statistics->transport.lbtrm.dgrams_dropped_hdr);
			printf("\tLBT-RM datagrams dropped (other)                          : %lu\n", Statistics->transport.lbtrm.dgrams_dropped_other);
			printf("\tLBT-RM datagrams received out of order                    : %lu\n", Statistics->transport.lbtrm.out_of_order);
			break;

		case LBM_TRANSPORT_STAT_LBTRU:
			printf("\tLBT-RU datagrams received                                 : %lu\n", Statistics->transport.lbtru.msgs_rcved);
			printf("\tLBT-RU datagram bytes received                            : %lu\n", Statistics->transport.lbtru.bytes_rcved);
			printf("\tLBT-RU NAK packets sent                                   : %lu\n", Statistics->transport.lbtru.nak_pckts_sent);
			printf("\tLBT-RU NAKs sent                                          : %lu\n", Statistics->transport.lbtru.naks_sent);
			printf("\tLost LBT-RU datagrams detected                            : %lu\n", Statistics->transport.lbtru.lost);
			printf("\tNCFs received (ignored)                                   : %lu\n", Statistics->transport.lbtru.ncfs_ignored);
			printf("\tNCFs received (shed)                                      : %lu\n", Statistics->transport.lbtru.ncfs_shed);
			printf("\tNCFs received (retransmit delay)                          : %lu\n", Statistics->transport.lbtru.ncfs_rx_delay);
			printf("\tNCFs received (unknown)                                   : %lu\n", Statistics->transport.lbtru.ncfs_unknown);
			printf("\tLoss recovery minimum time                                : %lums\n", Statistics->transport.lbtru.nak_stm_min);
			printf("\tLoss recovery mean time                                   : %lums\n", Statistics->transport.lbtru.nak_stm_mean);
			printf("\tLoss recovery maximum time                                : %lums\n", Statistics->transport.lbtru.nak_stm_max);
			printf("\tMinimum transmissions per individual NAK                  : %lu\n", Statistics->transport.lbtru.nak_tx_min);
			printf("\tMean transmissions per individual NAK                     : %lu\n", Statistics->transport.lbtru.nak_tx_mean);
			printf("\tMaximum transmissions per individual NAK                  : %lu\n", Statistics->transport.lbtru.nak_tx_max);
			printf("\tDuplicate LBT-RU datagrams received                       : %lu\n", Statistics->transport.lbtru.duplicate_data);
			printf("\tLBT-RU datagrams unrecoverable (window advance)           : %lu\n", Statistics->transport.lbtru.unrecovered_txw);
			printf("\tLBT-RU datagrams unrecoverable (NAK generation expiration): %lu\n", Statistics->transport.lbtru.unrecovered_tmo);
			printf("\tLBT-RU LBM messages received                              : %lu\n", Statistics->transport.lbtru.lbm_msgs_rcved);
			printf("\tLBT-RU LBM messages received with uninteresting topic     : %lu\n", Statistics->transport.lbtru.lbm_msgs_no_topic_rcved);
			printf("\tLBT-RU LBM requests received                              : %lu\n", Statistics->transport.lbtru.lbm_reqs_rcved);
			printf("\tLBT-RU datagrams dropped (size)                           : %lu\n", Statistics->transport.lbtru.dgrams_dropped_size);
			printf("\tLBT-RU datagrams dropped (type)                           : %lu\n", Statistics->transport.lbtru.dgrams_dropped_type);
			printf("\tLBT-RU datagrams dropped (version)                        : %lu\n", Statistics->transport.lbtru.dgrams_dropped_version);
			printf("\tLBT-RU datagrams dropped (hdr)                            : %lu\n", Statistics->transport.lbtru.dgrams_dropped_hdr);
			printf("\tLBT-RU datagrams dropped (SID)                            : %lu\n", Statistics->transport.lbtru.dgrams_dropped_sid);
			printf("\tLBT-RU datagrams dropped (other)                          : %lu\n", Statistics->transport.lbtru.dgrams_dropped_other);
			break;

		case LBM_TRANSPORT_STAT_LBTIPC:
			printf("\tLBT-IPC datagrams received                                : %lu\n", Statistics->transport.lbtipc.msgs_rcved);
			printf("\tLBT-IPC datagram bytes received                           : %lu\n", Statistics->transport.lbtipc.bytes_rcved);
			printf("\tLBT-IPC LBM messages received                             : %lu\n", Statistics->transport.lbtipc.lbm_msgs_rcved);
			printf("\tLBT-IPC LBM messages received with uninteresting topic    : %lu\n", Statistics->transport.lbtipc.lbm_msgs_no_topic_rcved);
			printf("\tLBT-IPC LBM requests received                             : %lu\n", Statistics->transport.lbtipc.lbm_reqs_rcved);
			break;

		case LBM_TRANSPORT_STAT_LBTSMX:
			printf("\tLBT-SMX datagrams received                                : %lu\n", Statistics->transport.lbtsmx.msgs_rcved);
			printf("\tLBT-SMX datagram bytes received                           : %lu\n", Statistics->transport.lbtsmx.bytes_rcved);
			printf("\tLBT-SMX LBM messages received                             : %lu\n", Statistics->transport.lbtsmx.lbm_msgs_rcved);
			printf("\tLBT-SMX LBM messages received with uninteresting topic    : %lu\n", Statistics->transport.lbtsmx.lbm_msgs_no_topic_rcved);
			printf("\tLBT-SMX LBM requests received                             : %lu\n", Statistics->transport.lbtsmx.lbm_reqs_rcved);
			break;

		case LBM_TRANSPORT_STAT_LBTRDMA:
			printf("\tLBT-RDMA datagrams received              : %lu\n", Statistics->transport.lbtrdma.msgs_rcved);
			printf("\tLBT-RDMA datagram bytes received         : %lu\n", Statistics->transport.lbtrdma.bytes_rcved);
			printf("\tLBT-RDMA LBM messages received           : %lu\n", Statistics->transport.lbtrdma.lbm_msgs_rcved);
			printf("\tLBT-RDMA LBM messages received (no topic): %lu\n", Statistics->transport.lbtrdma.lbm_msgs_no_topic_rcved);
			printf("\tLBT-RDMA LBM requests received           : %lu\n", Statistics->transport.lbtrdma.lbm_reqs_rcved);
			break;
	}
	fflush(stdout);
}

void src_statistics_cb(const void * AttributeBlock, const lbm_src_transport_stats_t * Statistics, void * ClientData)
{
	lbm_ulong_t source = LBMMON_ATTR_SOURCE_NORMAL;

	if (lbmmon_attr_get_source(AttributeBlock, &source) != 0)
	{
		source = LBMMON_ATTR_SOURCE_NORMAL;
	}
	switch (source)
	{
		case LBMMON_ATTR_SOURCE_IM:
			print_attributes("Context IM source statistics received", AttributeBlock);
			break;
		default:
			print_attributes("Source statistics received", AttributeBlock);
			break;
	}
	printf("Source: %s\n", Statistics->source);
	printf("Transport: %s\n", translate_transport(Statistics->type));
	switch (Statistics->type)
	{
		case LBM_TRANSPORT_STAT_TCP:
			printf("\tClients       : %lu\n", Statistics->transport.tcp.num_clients);
			printf("\tBytes buffered: %lu\n", Statistics->transport.tcp.bytes_buffered);
			break;

		case LBM_TRANSPORT_STAT_LBTRM:
			printf("\tLBT-RM datagrams sent                                 : %lu\n", Statistics->transport.lbtrm.msgs_sent);
			printf("\tLBT-RM datagram bytes sent                            : %lu\n", Statistics->transport.lbtrm.bytes_sent);
			printf("\tLBT-RM datagrams in transmission window               : %lu\n", Statistics->transport.lbtrm.txw_msgs);
			printf("\tLBT-RM datagram bytes in transmission window          : %lu\n", Statistics->transport.lbtrm.txw_bytes);
			printf("\tLBT-RM NAK packets received                           : %lu\n", Statistics->transport.lbtrm.nak_pckts_rcved);
			printf("\tLBT-RM NAKs received                                  : %lu\n", Statistics->transport.lbtrm.naks_rcved);
			printf("\tLBT-RM NAKs ignored                                   : %lu\n", Statistics->transport.lbtrm.naks_ignored);
			printf("\tLBT-RM NAKs shed                                      : %lu\n", Statistics->transport.lbtrm.naks_shed);
			printf("\tLBT-RM NAKs ignored (retransmit delay)                : %lu\n", Statistics->transport.lbtrm.naks_rx_delay_ignored);
			printf("\tLBT-RM retransmission datagrams sent                  : %lu\n", Statistics->transport.lbtrm.rxs_sent);
			printf("\tLBT-RM datagrams queued by rate control               : %lu\n", Statistics->transport.lbtrm.rctlr_data_msgs);
			printf("\tLBT-RM retransmission datagrams queued by rate control: %lu\n", Statistics->transport.lbtrm.rctlr_rx_msgs);
			printf("\tLBT-RM retransmission datagram bytes sent             : %lu\n", Statistics->transport.lbtrm.rx_bytes_sent);
			break;

		case LBM_TRANSPORT_STAT_LBTRU:
			printf("\tLBT-RU datagrams sent                    : %lu\n", Statistics->transport.lbtru.msgs_sent);
			printf("\tLBT-RU datagram bytes sent               : %lu\n", Statistics->transport.lbtru.bytes_sent);
			printf("\tLBT-RU NAK packets received              : %lu\n", Statistics->transport.lbtru.nak_pckts_rcved);
			printf("\tLBT-RU NAKs received                     : %lu\n", Statistics->transport.lbtru.naks_rcved);
			printf("\tLBT-RU NAKs ignored                      : %lu\n", Statistics->transport.lbtru.naks_ignored);
			printf("\tLBT-RU NAKs shed                         : %lu\n", Statistics->transport.lbtru.naks_shed);
			printf("\tLBT-RU NAKs ignored (retransmit delay)   : %lu\n", Statistics->transport.lbtru.naks_rx_delay_ignored);
			printf("\tLBT-RU retransmission datagrams sent     : %lu\n", Statistics->transport.lbtru.rxs_sent);
			printf("\tClients                                  : %lu\n", Statistics->transport.lbtru.num_clients);
			printf("\tLBT-RU retransmission datagram bytes sent: %lu\n", Statistics->transport.lbtru.rx_bytes_sent);
			break;

		case LBM_TRANSPORT_STAT_LBTIPC:
			printf("\tClients                    : %lu\n", Statistics->transport.lbtipc.num_clients);
			printf("\tLBT-IPC datagrams sent     : %lu\n", Statistics->transport.lbtipc.msgs_sent);
			printf("\tLBT-IPC datagram bytes sent: %lu\n", Statistics->transport.lbtipc.bytes_sent);
			break;

		case LBM_TRANSPORT_STAT_LBTSMX:
			printf("\tClients                   : %lu\n", Statistics->transport.lbtsmx.num_clients);
			printf("\tLBT-SMX datagrams sent     : %lu\n", Statistics->transport.lbtsmx.msgs_sent);
			printf("\tLBT-SMX datagram bytes sent: %lu\n", Statistics->transport.lbtsmx.bytes_sent);
			break;

		case LBM_TRANSPORT_STAT_LBTRDMA:
			printf("\tClients                    : %lu\n", Statistics->transport.lbtrdma.num_clients);
			printf("\tLBT-RDMA datagrams sent     : %lu\n", Statistics->transport.lbtrdma.msgs_sent);
			printf("\tLBT-RDMA datagram bytes sent: %lu\n", Statistics->transport.lbtrdma.bytes_sent);
			break;
	}
	fflush(stdout);
}

void evq_statistics_cb(const void * AttributeBlock, const lbm_event_queue_stats_t * Statistics, void * ClientData)
{
	print_attributes("Event queue statistics received", AttributeBlock);
	printf("\tData messages currently enqueued                : %lu\n", Statistics->data_msgs);
	printf("\tTotal data messages enqueued                    : %lu\n", Statistics->data_msgs_tot);
	printf("\tData message service minimum time               : %luus\n", Statistics->data_msgs_svc_min);
	printf("\tData message service mean time                  : %luus\n", Statistics->data_msgs_svc_mean);
	printf("\tData message service maximum time               : %luus\n", Statistics->data_msgs_svc_max);
	printf("\tResponses currently enqueued                    : %lu\n", Statistics->resp_msgs);
	printf("\tTotal responses enqueued                        : %lu\n", Statistics->resp_msgs_tot);
	printf("\tResponse service minimum time                   : %luus\n", Statistics->resp_msgs_svc_min);
	printf("\tResponse service mean time                      : %luus\n", Statistics->resp_msgs_svc_mean);
	printf("\tResponse service maximum time                   : %luus\n", Statistics->resp_msgs_svc_max);
	printf("\tTopicless immediate messages currently enqueued : %lu\n", Statistics->topicless_im_msgs);
	printf("\tTotal topicless immediate messages enqueued     : %lu\n", Statistics->topicless_im_msgs_tot);
	printf("\tTopicless immediate message service minimum time: %luus\n", Statistics->topicless_im_msgs_svc_min);
	printf("\tTopicless immediate message service mean time   : %luus\n", Statistics->topicless_im_msgs_svc_mean);
	printf("\tTopicless immediate message service maximum time: %luus\n", Statistics->topicless_im_msgs_svc_max);
	printf("\tWildcard messages currently enqueued            : %lu\n", Statistics->wrcv_msgs);
	printf("\tTotal wildcard messages enqueued                : %lu\n", Statistics->wrcv_msgs_tot);
	printf("\tWildcard message service minimum time           : %luus\n", Statistics->wrcv_msgs_svc_min);
	printf("\tWildcard message service mean time              : %luus\n", Statistics->wrcv_msgs_svc_mean);
	printf("\tWildcard message service maximum time           : %luus\n", Statistics->wrcv_msgs_svc_max);
	printf("\tI/O events currently enqueued                   : %lu\n", Statistics->io_events);
	printf("\tTotal I/O events enqueued                       : %lu\n", Statistics->io_events_tot);
	printf("\tI/O event service minimum time                  : %luus\n", Statistics->io_events_svc_min);
	printf("\tI/O event service mean time                     : %luus\n", Statistics->io_events_svc_mean);
	printf("\tI/O event service maximum time                  : %luus\n", Statistics->io_events_svc_max);
	printf("\tTimer events currently enqueued                 : %lu\n", Statistics->timer_events);
	printf("\tTotal timer events enqueued                     : %lu\n", Statistics->timer_events_tot);
	printf("\tTimer event service minimum time                : %luus\n", Statistics->timer_events_svc_min);
	printf("\tTimer event service mean time                   : %luus\n", Statistics->timer_events_svc_mean);
	printf("\tTimer event service maximum time                : %luus\n", Statistics->timer_events_svc_max);
	printf("\tSource events currently enqueued                : %lu\n", Statistics->source_events);
	printf("\tTotal source events enqueued                    : %lu\n", Statistics->source_events_tot);
	printf("\tSource event service minimum time               : %luus\n", Statistics->source_events_svc_min);
	printf("\tSource event service mean time                  : %luus\n", Statistics->source_events_svc_mean);
	printf("\tSource event service maximum time               : %luus\n", Statistics->source_events_svc_max);
	printf("\tUnblock events currently enqueued               : %lu\n", Statistics->unblock_events);
	printf("\tTotal unblock events enqueued                   : %lu\n", Statistics->unblock_events_tot);
	printf("\tCancel events currently enqueued                : %lu\n", Statistics->cancel_events);
	printf("\tTotal cancel events enqueued                    : %lu\n", Statistics->cancel_events_tot);
	printf("\tCancel event service minimum time               : %luus\n", Statistics->cancel_events_svc_min);
	printf("\tCancel event service mean time                  : %luus\n", Statistics->cancel_events_svc_mean);
	printf("\tCancel event service maximum time               : %luus\n", Statistics->cancel_events_svc_max);
	printf("\tCallback events currently enqueued              : %lu\n", Statistics->callback_events);
	printf("\tTotal callback events enqueued                  : %lu\n", Statistics->callback_events_tot);
	printf("\tCallback event service minimum time             : %luus\n", Statistics->callback_events_svc_min);
	printf("\tCallback event service mean time                : %luus\n", Statistics->callback_events_svc_mean);
	printf("\tCallback event service maximum time             : %luus\n", Statistics->callback_events_svc_max);
	printf("\tContext source events currently enqueued        : %lu\n", Statistics->context_source_events);
	printf("\tTotal context source events enqueued            : %lu\n", Statistics->context_source_events_tot);
	printf("\tContext source event service minimum time       : %luus\n", Statistics->context_source_events_svc_min);
	printf("\tContext source event service mean time          : %luus\n", Statistics->context_source_events_svc_mean);
	printf("\tContext source event service maximum time       : %luus\n", Statistics->context_source_events_svc_max);
	printf("\tEvents currently enqueued                       : %lu\n", Statistics->events);
	printf("\tTotal events enqueued                           : %lu\n", Statistics->events_tot);
	printf("\tEvent latency minimum time                      : %luus\n", Statistics->age_min);
	printf("\tEvent latency mean time                         : %luus\n", Statistics->age_mean);
	printf("\tEvent latency maximum time                      : %luus\n", Statistics->age_max);
	fflush(stdout);
}

void ctx_statistics_cb(const void * AttributeBlock, const lbm_context_stats_t * Statistics, void * ClientData)
{
	print_attributes("Context statistics received", AttributeBlock);
	printf("\tTopic resolution datagrams sent                    : %lu\n", Statistics->tr_dgrams_sent);
	printf("\tTopic resolution datagram bytes sent               : %lu\n", Statistics->tr_bytes_sent);
	printf("\tTopic resolution datagrams received                : %lu\n", Statistics->tr_dgrams_rcved);
	printf("\tTopic resolution datagram bytes received           : %lu\n", Statistics->tr_bytes_rcved);
	printf("\tTopic resolution datagrams dropped (version)       : %lu\n", Statistics->tr_dgrams_dropped_ver);
	printf("\tTopic resolution datagrams dropped (type)          : %lu\n", Statistics->tr_dgrams_dropped_type);
	printf("\tTopic resolution datagrams dropped (malformed)     : %lu\n", Statistics->tr_dgrams_dropped_malformed);
	printf("\tTopic resolution send failures                     : %lu\n", Statistics->tr_dgrams_send_failed);
	printf("\tTopics in source topic map                         : %lu\n", Statistics->tr_src_topics);
	printf("\tTopics in receiver topic map                       : %lu\n", Statistics->tr_rcv_topics);
	printf("\tUnresolved topics in receiver topic map            : %lu\n", Statistics->tr_rcv_unresolved_topics);
	printf("\tUnknown LBT-RM datagrams received                  : %lu\n", Statistics->lbtrm_unknown_msgs_rcved);
	printf("\tUnknown LBT-RU datagrams received                  : %lu\n", Statistics->lbtru_unknown_msgs_rcved);
	printf("\tNumber of times message send blocked               : %lu\n", Statistics->send_blocked);
	printf("\tNumber of times message send returned EWOULDBLOCK  : %lu\n", Statistics->send_would_block);
	printf("\tNumber of times response send blocked              : %lu\n", Statistics->resp_blocked);
	printf("\tNumber of times response send returned EWOULDBLOCK : %lu\n", Statistics->resp_would_block);
	printf("\tNumber of duplicate UIM messages dropped           : %lu\n", Statistics->uim_dup_msgs_rcved);
	printf("\tNumber of UIM messages received without stream info: %lu\n", Statistics->uim_msgs_no_stream_rcved);
	fflush(stdout);
}


void tmon_message_print(lbm_msg_t *msg)
{
	char msg_c_str[3*TMON_STR_BUF_LENS + 1];
    int data_len = msg->len;

	if (data_len >= sizeof(msg_c_str)) {
		printf("Error, tmon message too big (%ld), truncating\n", msg->len);
		data_len = sizeof(msg_c_str) - 1;
	}
	memcpy(msg_c_str, msg->data, data_len);
	msg_c_str[data_len] = '\0';

	switch (msg_c_str[0]) {
		case 'S': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long tmon_src;
			long tv_sec;
			long tv_usec;
			char topic[TMON_STR_BUF_LENS];
			int null_ofs = 0;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "S,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx,"  /* tmon_src */
				"%" STR(TMON_STR_BUF_LENS) "[^\n]" /* topic */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &tmon_src,
				topic, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 8 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Source create\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  tmon_src=%llx\n"
						"  time=%s and %ld usec\n"
						"  topic=%s\n",
					app_id, ip, pid, ctx, tmon_src,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
					topic);
			}
		}
		break;

		case 's': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long tmon_src;
			long tv_sec;
			long tv_usec;
			int null_ofs;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "s,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx"   /* tmon_src */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &tmon_src,
				&null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 7 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Source delete\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  tmon_src=%llx\n"
						"  time=%s and %ld usec\n",
					app_id, ip, pid, ctx, tmon_src,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec);
			}
		}
		break;

		case 'R': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long tmon_rcv;
			long tv_sec;
			long tv_usec;
			char topic[TMON_STR_BUF_LENS];
			int null_ofs = 0;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "R,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx,"  /* tmon_rcv */
				"%" STR(TMON_STR_BUF_LENS) "[^\n]" /* topic */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &tmon_rcv,
				topic, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 8 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Receiver create\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  tmon_rcv=%llx\n"
						"  time=%s and %ld usec\n"
						"  topic=%s\n",
					app_id, ip, pid, ctx, tmon_rcv,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
					topic);
			}
		}
		break;

		case 'r': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long tmon_rcv;
			long tv_sec;
			long tv_usec;
			int null_ofs;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "r,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx"   /* tmon_rcv */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &tmon_rcv,
				&null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 7 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Receiver delete\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  tmon_rcv=%llx\n"
						"  time=%s and %ld usec\n",
					app_id, ip, pid, ctx, tmon_rcv,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec);
			}
		}
		break;

		case 'W': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long tmon_wrcv;
			long tv_sec;
			long tv_usec;
			char pattern[TMON_STR_BUF_LENS];
			int null_ofs = 0;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "W,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx,"  /* tmon_wrcv */
				"%" STR(TMON_STR_BUF_LENS) "[^\n]" /* pattern */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &tmon_wrcv,
				pattern, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 8 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Wildcard receiver create\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  tmon_wrcv=%llx\n"
						"  time=%s and %ld usec\n"
						"  pattern=%s\n",
					app_id, ip, pid, ctx, tmon_wrcv,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
					pattern);
			}
		}
		break;

		case 'w': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long tmon_wrcv;
			long tv_sec;
			long tv_usec;
			int null_ofs;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "w,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx"   /* tmon_wrcv */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &tmon_wrcv,
				&null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 7 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Wildcard receiver delete\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  tmon_wrcv=%llx\n"
						"  time=%s and %ld usec\n",
					app_id, ip, pid, ctx, tmon_wrcv,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec);
			}
		}
		break;

		case 'C': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long tmon_rcv;
			unsigned long long conn;
			long tv_sec;
			long tv_usec;
			char src_str[TMON_STR_BUF_LENS];
			int null_ofs = 0;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "C,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx,"  /* tmon_rcv */
				"%llx,"  /* conn */
				"%" STR(TMON_STR_BUF_LENS) "[^\n]" /* src_str */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &tmon_rcv, &conn,
				src_str, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 9 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Conn create\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  tmon_rcv=%llx\n"
						"  conn=%llx\n"
						"  time=%s and %ld usec\n"
						"  src_str=%s\n",
					app_id, ip, pid, ctx, tmon_rcv, conn,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
					src_str);
			}
		}
		break;

		case 'c': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long conn;
			long tv_sec;
			long tv_usec;
			long msg_count;
			long unrec_count;
			long burst_count;
			int null_ofs;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "c,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx,"  /* conn */
				"%lu,"   /* msg_count */
				"%lu,"   /* unrec_count */
				"%lu"    /* burst_count */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &conn,
				&msg_count, &unrec_count, &burst_count, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 10 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Conn delete\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  conn=%llx\n"
						"  time=%s and %ld usec\n"
						"  msg_count=%lu\n"
						"  unrec_count=%lu\n"
						"  burst_count=%lu\n",
					app_id, ip, pid, ctx, conn,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
					msg_count, unrec_count, burst_count);
			}
		}
		break;

		case 'B': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long conn;
			long tv_sec;
			long tv_usec;
			char topic[TMON_STR_BUF_LENS];
			int null_ofs = 0;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "B,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx,"  /* conn */
				"%" STR(TMON_STR_BUF_LENS) "[^\n]" /* topic */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &conn,
				topic, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 8 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("BOS\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  conn=%llx\n"
						"  time=%s and %ld usec\n"
						"  topic=%s\n",
					app_id, ip, pid, ctx, conn,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
                    topic);
			}
		}
		break;

		case 'E': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long conn;
			long tv_sec;
			long tv_usec;
			long msg_count;
			long unrec_count;
			long burst_count;
			char topic[TMON_STR_BUF_LENS];
			int null_ofs;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "E,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx,"  /* conn */
				"%lu,"   /* msg_count */
				"%lu,"   /* unrec_count */
				"%lu,"   /* burst_count */
				"%" STR(TMON_STR_BUF_LENS) "[^\n]" /* topic */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &conn,
				&msg_count, &unrec_count, &burst_count, topic, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 11 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("EOS\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  conn=%llx\n"
						"  time=%s and %ld usec\n"
						"  msg_count=%lu\n"
						"  unrec_count=%lu\n"
						"  burst_count=%lu\n"
						"  topic=%s\n",
					app_id, ip, pid, ctx, conn,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
					msg_count, unrec_count, burst_count, topic);
			}
		}
		break;

		case 'A': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			long tv_sec;
			long tv_usec;
			char err_str[TMON_STR_BUF_LENS];
			int null_ofs = 0;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "A,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%" STR(TMON_STR_BUF_LENS) "[^\n]"  /* err_str */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec,
				err_str, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 7 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Alarm\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  time=%s and %ld usec\n"
						"  err_str=%s\n",
					app_id, ip, pid, ctx,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
					err_str);
			}
		}
		break;

		case 'L': {
			int scanf_rtn;
			char app_id[TMON_STR_BUF_LENS];
			char ip[TMON_STR_BUF_LENS];
			unsigned long pid;
			unsigned long long ctx;
			unsigned long long conn;
			long tv_sec;
			long tv_usec;
			long msg_count;
			long unrec_count;
			long burst_count;
			int null_ofs;
			char asc_time[32];

			scanf_rtn = sscanf(msg_c_str, "L,"
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* app_id */
				"%" STR(TMON_STR_BUF_LENS) "[^,],"  /* ip */
				"%lx,"   /* pid */
				"%llx,"  /* ctx */
				"%lu,"   /* tv_sec */
				"%lu,"   /* tv_usec */
				"%llx,"  /* conn */
				"%lu,"   /* msg_count */
				"%lu,"   /* unrec_count */
				"%lu"    /* burst_count */
				"%n",    /* null_ofs */
				app_id, ip, &pid, &ctx, &tv_sec, &tv_usec, &conn,
				&msg_count, &unrec_count, &burst_count, &null_ofs);
			/* See http://blog.geeky-boy.com/2018/09/safe-sscanf-usage.html */
			if (scanf_rtn != 10 || null_ofs == 0 || msg_c_str[null_ofs] != '\0') {
				fprintf(stderr, "scanf_rtn=%d, null_ofs=%d, msg='%s'\n", scanf_rtn, null_ofs, msg_c_str); fflush(stderr);
			}
			else {
				printf("Loss\n"
						"  app_id=%s\n"
						"  ip=%s\n"
						"  pid=%lx\n"
						"  ctx=%llx\n"
						"  conn=%llx\n"
						"  time=%s and %ld usec\n"
						"  msg_count=%lu\n"
						"  unrec_count=%lu\n"
						"  burst_count=%lu\n",
					app_id, ip, pid, ctx, conn,
					tmon_ctime(asc_time, sizeof(asc_time), tv_sec), tv_usec,
					msg_count, unrec_count, burst_count);
			}
		}
		break;

		default:
			printf("Unrecognized msg=%s\n", msg_c_str);
	}
	fflush(stdout);
}  /* tmon_message_print */


int tmon_message_rcv(lbm_rcv_t *rcv, lbm_msg_t *msg, void *clientd)
{
	switch (msg->type) {
	case LBM_MSG_DATA:
		tmon_message_print(msg);
		break;
	default:
		printf("tmon_message_rcv: msg->type=%d\n", msg->type);
	}

	return 0;
}  /* tmon_message_rcv */


int main(int argc, char **argv)
{
	lbmmon_rctl_t * monctl;
	lbmmon_rctl_attr_t * attr;
	lbmmon_rcv_statistics_func_t rcv = { rcv_statistics_cb };
	lbmmon_src_statistics_func_t src = { src_statistics_cb };
	lbmmon_evq_statistics_func_t evq = { evq_statistics_cb };
	lbmmon_ctx_statistics_func_t ctx = { ctx_statistics_cb };
	int rc;
	int c;
	int errflag = 0;
	char * transport_options = NULL;
	char transport_options_string[1024];
	char * format_options = NULL;
	char format_options_string[1024];
	const lbmmon_transport_func_t * transport = lbmmon_transport_lbm_module();
	const lbmmon_format_func_t * format = lbmmon_format_csv_module();
	char topic_str[TMON_STR_BUF_LENS];
	char config_file[TMON_STR_BUF_LENS];
	lbm_context_t *tmon_ctx;
	lbm_rcv_t *tmon_rcv;

#ifdef _WIN32
	{
		WSADATA wsadata;
		int status;

		/* Windows socket setup code */
		if ((status = WSAStartup(MAKEWORD(2,2), &wsadata)) != 0)
		{
			fprintf(stderr, "%s: WSA startup error - %d\n", argv[0], status);
			exit(1);
		}
	}
#else
	/*
	 * Ignore SIGPIPE on UNIXes which can occur when writing to a socket
	 * with only one open end point.
	 */
	signal(SIGPIPE, SIG_IGN);
#endif /* _WIN32 */

	lbm_log(log_callback, NULL);

	memset(transport_options_string, 0, sizeof(transport_options_string));
	memset(format_options_string, 0, sizeof(format_options_string));

	while ((c = getopt_long(argc, argv, OptionString, OptionTable, NULL)) != EOF)
	{
		switch (c)
		{
			case 'c':
				/* Initialize configuration parameters from a file. */
				if (lbm_config(optarg) == LBM_FAILURE) {
					fprintf(stderr, "lbm_config: %s\n", lbm_errmsg());
					exit(1);
				}
				break;
			case 't':
				if (optarg != NULL)
				{
					if (strcasecmp(optarg, "lbm") == 0)
					{
						transport = lbmmon_transport_lbm_module();
					}
					else if (strcasecmp(optarg, "udp") == 0)
					{
						transport = lbmmon_transport_udp_module();
					}
					else if (strcasecmp(optarg, "lbmsnmp") == 0)
					{
						transport = lbmmon_transport_lbmsnmp_module();
					}
					else
					{
						++errflag;
					}
				}
				else
				{
					++errflag;
				}
				break;

			case 0:
				if (optarg != NULL)
				{
					strncpy(transport_options_string, optarg, sizeof(transport_options_string));
				}
				else
				{
					++errflag;
				}
				break;

			case 'f':
				if (optarg != NULL)
				{
					if (strcasecmp(optarg, "csv") == 0)
					{
						format = lbmmon_format_csv_module();
					}
					else
					{
						++errflag;
					}
				}
				else
				{
					++errflag;
				}
				break;

			case 1:
				if (optarg != NULL)
				{
					strncpy(format_options_string, optarg, sizeof(format_options_string));
				}
				else
				{
					++errflag;
				}
				break;

			case 'h':
				fprintf(stderr, "%s\n%s\n", lbm_version(), Purpose);
				fprintf(stderr, Usage, argv[0]);
				exit(0);

			case '?':
			default:
				++errflag;
				break;
		}
	}

	if (errflag != 0)
	{
		fprintf(stderr, "%s\n", lbm_version());
		fprintf(stderr, Usage, argv[0]);
		exit(1);
	}

	if (strlen(transport_options_string) > 0)
	{
		transport_options = transport_options_string;
	}
	if (strlen(format_options_string) > 0)
	{
		format_options = format_options_string;
	}

	rc = lbmmon_rctl_attr_create(&attr);
	if (attr == NULL)
	{
		fprintf(stderr, "call to lbmmon_rctl_attr_create() failed, %s\n", lbmmon_errmsg());
		exit(1);
	}
	rc = lbmmon_rctl_attr_setopt(attr, LBMMON_RCTL_RECEIVER_CALLBACK, (void *) &rcv, sizeof(rcv));
	if (rc != 0)
	{
		fprintf(stderr, "call to lbmmon_rctl_attr_setopt() failed, %s\n", lbmmon_errmsg());
		exit(1);
	}
	rc = lbmmon_rctl_attr_setopt(attr, LBMMON_RCTL_SOURCE_CALLBACK, (void *) &src, sizeof(src));
	if (rc != 0)
	{
		fprintf(stderr, "call to lbmmon_rctl_attr_setopt() failed, %s\n", lbmmon_errmsg());
		exit(1);
	}
	rc = lbmmon_rctl_attr_setopt(attr, LBMMON_RCTL_EVENT_QUEUE_CALLBACK, (void *) &evq, sizeof(evq));
	if (rc != 0)
	{
		fprintf(stderr, "call to lbmmon_rctl_attr_setopt() failed, %s\n", lbmmon_errmsg());
		exit(1);
	}
	rc = lbmmon_rctl_attr_setopt(attr, LBMMON_RCTL_CONTEXT_CALLBACK, (void *) &ctx, sizeof(ctx));
	if (rc != 0)
	{
		fprintf(stderr, "call to lbmmon_rctl_attr_setopt() failed, %s\n", lbmmon_errmsg());
		exit(1);
	}
	rc = lbmmon_rctl_create(&monctl, format, (void *) format_options, transport, (void *) transport_options, attr, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "call to lbmmon_rctl_create() failed, %s\n", lbmmon_errmsg());
		exit(1);
	}
	lbmmon_rctl_attr_delete(attr);

	tmon_ctx = tmon_create_context(topic_str, config_file, TMON_STR_BUF_LENS,
	  transport_options);
	tmon_rcv = tmon_create_monrcv(tmon_ctx, topic_str, config_file,
	  transport_options, tmon_message_rcv, NULL, NULL);

	while (1)
	{
		SLEEP_SEC(2);
	}

	lbm_rcv_delete(tmon_rcv);
	lbm_context_delete(tmon_ctx);

	lbmmon_rctl_destroy(monctl);
	return (0);
}

int log_callback(int Level, const char * Message, void * ClientData)
{
	fprintf(stderr, "%s\n", Message);
	return (0);
}

