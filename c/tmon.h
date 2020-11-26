/* tmon.h - Rate Limiter header file.
 * Project home: https://github.com/UltraMessaging/tmon
 *
 * Copyright (c) 2020 Informatica Corporation. All Rights Reserved.
 * Permission is granted to licensees to use
 * or alter this software for any purpose, including commercial applications,
 * according to the terms laid out in the Software License Agreement.
 *
 * This source code example is provided by Informatica for educational
 * and evaluation purposes only.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INFORMATICA DISCLAIMS ALL WARRANTIES
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF
 * NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR
 * PURPOSE.  INFORMATICA DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE
 * UNINTERRUPTED OR ERROR-FREE.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES,
 * BE LIABLE TO LICENSEE FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR
 * INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGREEMENT OR THE
 * TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BEEN APPRISED OF
 * THE LIKELIHOOD OF SUCH DAMAGES.
 */

#ifndef TMON_H
#define TMON_H

#if defined(_WIN32)
#include <Windows.h>
#include <stdint.h>

#else  /* unix */
#include <sys/time.h>
#include <time.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#define TMON_STR_BUF_LENS 512

typedef struct ezstr_s {
  char *buff;
  size_t alloc_size;
  size_t null_index;
} ezstr_t;
  

/* Structure for "tmon" object. Create right after creating app context.
 * App should mostly treat it as opaque (ignore it). */
typedef struct tmon_s {
  lbm_context_t *app_ctx;           /* Application context. */
  char transport_opts[2048];        /* Monitoring transport options string. */
  char app_id[TMON_STR_BUF_LENS];   /* Monitoring config file. */
  char config_file[TMON_STR_BUF_LENS];  /* Monitoring config file. */
  char topic_str[TMON_STR_BUF_LENS];    /* Topic string for tmon source. */
  lbm_context_t *ctx;               /* Context for publishing tmon data. */
  lbm_src_t *src;                   /* Source for publishing tmon data. */

  ezstr_t *header;                 /* String for header. */
} tmon_t;


/* Structure for "tmon_rcv" object. Create right *before* creating each app
 * receiver. App should mostly treat it as opaque. */
typedef struct tmon_rcv_s {
  tmon_t *tmon;              /* Parent tmon object. */
  char app_topic_str[TMON_STR_BUF_LENS];   /* Topic name for app rcv. */
  struct timeval create_time;
  ezstr_t *mon_msg;              /* String for building monitoring message. */
} tmon_rcv_t;


/* Structure for "tmon_src" object. Create right *before* creating each app
 * source. App should mostly treat it as opaque. */
typedef struct tmon_src_s {
  tmon_t *tmon;              /* Parent tmon object. */
  char app_topic_str[256];   /* Topic name for app receiver to be created. */
  struct timeval create_time;
  ezstr_t *mon_msg;         /* String for building monitoring message. */
} tmon_src_t;


/* Structure for "tmon_conn" object. Create inside source notification
 * create callback function (source_notification_function). App should mostly
 * treat it as opaque. */
typedef struct tmon_conn_s {
  tmon_rcv_t *tmon_rcv;
  char source_str[128];    /* Source string for transport session, incl tidx. */
  struct timeval create_time;
  struct timeval bos_time;
  struct timeval loss_time;
  long msg_count;
  long unrec_count;
  long burst_count;
  ezstr_t *mon_msg;       /* String for building monitoring message. */
} tmon_conn_t;


void tmon_gettimeofday(struct timeval *tv);
const char *tmon_ctime(char *buffer, size_t bufsize, time_t cur_time);
char *tmon_inet_ntop(lbm_uint_t addr, char *dst, size_t buf_len);
void tmon_usleep(int usec);

tmon_t *tmon_create(lbm_context_t *app_ctx);
void tmon_delete(tmon_t *tmon);

tmon_rcv_t *tmon_rcv_create(tmon_t *tmon, char *app_topic_str);
void tmon_rcv_delete(tmon_rcv_t *tmon_rcv);
tmon_src_t *tmon_src_create(tmon_t *tmon, char *app_topic_str);
void tmon_src_delete(tmon_src_t *tmon_rcv);

tmon_conn_t *tmon_conn_create(tmon_rcv_t *tmon_rcv, const char *source_str);
void tmon_conn_delete(tmon_conn_t *tmon_conn);
void tmon_conn_rcv_event(lbm_msg_t *msg);

lbm_context_t *tmon_create_context(char *topic_str, char *config_file, int buf_lens, char *transport_opts);
lbm_rcv_t *tmon_create_monrcv(lbm_context_t *ctx, const char *topic_str, const char *config_file, const char *transport_opts, lbm_rcv_cb_proc proc, void *clientd, lbm_event_queue_t *evq);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif  /* TMON_H */
