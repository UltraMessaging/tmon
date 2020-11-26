/*! \file tmon.h
  \brief UM Topic Monitor API.
Please read \ref disclaimers before using this software.
See \ref overview for introduction and user manual.

See <a href="https://github.com/UltraMessaging/tmon">https://github.com/UltraMessaging/tmon</a>
for code and documentation.

*/

/*
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

#include "lbm/lbm.h"

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

/*! \brief max length of various strings.
 */
#define TMON_STR_BUF_LENS 512

/*! \brief Simple, "easy" string object, used internally by tmon.

  Not intended as a supported API.
 */
typedef struct ezstr_s {
  char *buff;
  size_t alloc_size;
  size_t null_index;
} ezstr_t;
  

/*! \brief top-level tmon object.

  Create right after creating the application context.
 */
typedef struct tmon_s {
  /*! \brief Application UM context. */
  lbm_context_t *app_ctx;
  /*! \brief Monitoring transport options string.

    Extracted from application context. */
  char transport_opts[2048];
  /*! Monitoring configuration file.

    Extracted from application context. */
  char app_id[TMON_STR_BUF_LENS];
  char config_file[TMON_STR_BUF_LENS];  /* Monitoring config file. */
  char topic_str[TMON_STR_BUF_LENS];    /* Topic string for tmon source. */
  lbm_context_t *ctx;               /* Context for publishing tmon data. */
  lbm_src_t *src;                   /* Source for publishing tmon data. */

  ezstr_t *header;                 /* String for header. */
} tmon_t;


/*! \brief tmon receiver monitoring object.

  Create one right *before* creating each app receiver.
 */
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


/*! \brief Portable microsecond-precision time stamp.

 \param usec Pointer to a struct timeval.
 */
void tmon_gettimeofday(struct timeval *tv);

/*! \brief Portable, reentrant converter from Posix "time_t" to ascii date/time.

  Unix "time_t" is number of seconds since the Unix Epoch (midnight, 1-Jan-1970).

  \param buffer Byte array at least 27 bytes long.
    Date/time string will be written here.
  \param bufsize Actual number of bytes in "buffer".
  \param cur_time Posix time value to convert.
 */
const char *tmon_ctime(char *buffer, size_t bufsize, time_t cur_time);

/*! \brief Portable, reentrant converter from binary IPv4 address to ascii.

  \param addr 32-bit binary form of IPv4 address,
  as would be present in struct sockaddr_in.sin_addr.s_addr.
  \param dst Byte array at least 16 bytes long.
    Dotted numeric IP address will be written here.
  \param buf_len Actual size of "dst" buffer.
  \return pointer to input buffer (returned for convenience).
 */
char *tmon_inet_ntop(lbm_uint_t addr, char *dst, size_t buf_len);

/*! \brief Portable microsecond-precision sleep function.

  Note that there is no guarantee of accuracy.
  Many operating systems do not support sleep times under a millisecond.
  User of this function should be prepared to experience much longer
  sleep times than requested.

  \param usec number of microseconds to sleep.
 */
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
