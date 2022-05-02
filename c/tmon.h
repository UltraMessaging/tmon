/*! \file tmon.h
 * \brief UM Topic Monitor API.
 *
 * Project home: <a href="https://github.com/UltraMessaging/tmon">https://github.com/UltraMessaging/tmon</a>
 *
 * Please read \ref disclaimers before using this software.
 * See \ref overview for introduction and user manual.
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

/*! \brief receiver type for regular (non-wildcard).
 */
#define TMON_RCV_TYPE_REGULAR 1
#define TMON_RCV_TYPE_WILDCARD 2

/*! \brief Portable atomic increment. Returns the incremented value.
 */
#if defined(_WIN32)
__inline static LONG tmon_inline_atomic_long_incr(LONG *value)
#elif defined(__TANDEM)
inline static long tmon_inline_atomic_long_incr(volatile long *value)
#else
__inline static long tmon_inline_atomic_long_incr(volatile long *value)
#endif
{
#if defined(_WIN32)
  return InterlockedIncrement(value);
#elif defined(__xlc__)
  return  __atomic_add_fetch(value,1,__ATOMIC_RELAXED);
#elif defined(HAVE_GCC_SYNC_ADD_AND_FETCH)
  return __sync_add_and_fetch(value, 1);
#elif defined(__GNUC__) && defined(HAVE_PENTIUM_PROCESSOR)
  long tmp = 1;
  unsigned long addr = (unsigned long)value;
  __asm__ volatile ("lock ; xadd %0, (%1)" : "+r"(tmp) : "r"(addr) : "memory");
  return tmp + 1;
#elif defined(__VMS)
  /* builtin returns old value. We want new value so add 1 */
  return __ATOMIC_INCREMENT_LONG(value) + 1;
#elif defined(SOLARIS) && defined(HAVE_SYS_ATOMIC_H)
  return atomic_inc_ulong_nv((ulong_t *) value);
#else
  #error Unrecognized platform.
#endif
}  /* tmon_inline_atomic_long_incr */


/*! \brief Simple, "easy" string object, used internally by tmon.
 * Not intended as a supported API.
 */
typedef struct ezstr_s {
  /*! Malloced buffer for string storage. */
  char *buff;
  /*! Size of malloced buffer. */
  size_t alloc_size;
  /*! Array index of string's trailing NULL. Also the number of characters
   * in the string. */
  size_t null_index;
} ezstr_t;
  

/*! \brief tmon context monitoring object.
 * Created by tmon_ctx_create().
 * Create one of these per application context immediately
 * *after* creating each context.
 * Application must not write to fields in this structure.
 *
 * Methods: tmon_ctx_create(), tmon_ctx_delete().
 */
typedef struct tmon_ctx_s {
  /*! \brief Application UM context. */
  lbm_context_t *app_ctx;
  /*! \brief Monitoring transport options string, as defined by application's
   * <a href="https://ultramessaging.github.io/currdoc/doc/Config/grpautomaticmonitoring.html#monitortransportoptscontext">monitor_transport_opts (context)</a>.
   *
   * Extracted from application context. */
  char transport_opts[2048];
  /*! Monitoring application ID, as defined by application's
   * <a href="https://ultramessaging.github.io/currdoc/doc/Config/grpautomaticmonitoring.html#monitorappidcontext">monitor_appid (context)</a>.
   *
   * Extracted from application context. */
  char app_id[TMON_STR_BUF_LENS];
  /*! Monitoring configuration file, as supplied by the "config" setting within the
   * <a href="https://ultramessaging.github.io/currdoc/doc/Config/grpautomaticmonitoring.html#monitortransportoptscontext">monitor_transport_opts (context)</a>
   * configuration option.
   * This setting is optional.
   *
   * Extracted from application context. */
  char config_file[TMON_STR_BUF_LENS];
  /*! Monitoring topic, as supplied by the "tmon_topic" setting within the
   * <a href="https://ultramessaging.github.io/currdoc/doc/Config/grpautomaticmonitoring.html#monitortransportoptscontext">monitor_transport_opts (context)</a>
   * configuration option.
   * This setting is optional and defaults to "/29west/tmon".
   *
   * Extracted from application context. */
  char topic_str[TMON_STR_BUF_LENS];
  /*! Suppression time interval in seconds after reporting an
   * unrecoverable loss or unrecoverable burst loss event before another
   * report is allowed.
   * Any loss events during the interval is counted in the stats,
   * but does not generate a report to the monitoring tool.
   * Defaults to 1 second. */
  int tmon_loss_suppress;
  /*! UM context created for sending tmon messages. */
  lbm_context_t *ctx;
  /*! UM source created for sending tmon messages. */
  lbm_src_t *src;
  /*! String to hold tmon message header information. */
  ezstr_t *header;
} tmon_ctx_t;


/*! \brief tmon receiver or wildcard receiver monitoring object.
 * Created by tmon_rcv_create().
 * Create one per application receiver immediately
 * *before* creating each receiver.
 * Application must not write to fields in this structure.
 *
 * Methods: tmon_rcv_create(), tmon_rcv_delete().
 */
typedef struct tmon_rcv_s {
  /*! \brief Parent tmon context monitoring object. */
  tmon_ctx_t *tmon_ctx;
  /*! Receiver type (1=topic, 2=wildcard). */
  int rcv_type;
  /*! Topic name or pattern for application receiver. */
  char app_topic_str[TMON_STR_BUF_LENS];
  /*! Time stamp when this object was created. */
  struct timeval create_time;
  /*! Work buffer for building tmon messages.
   * This is retained to minimize repeated malloc/free. */
  ezstr_t *mon_msg;
} tmon_rcv_t;


/*! \brief tmon source monitoring object.
 * Created by tmon_src_create().
 * Create one per application source immediately
 * *before* creating each source.
 * Application must not write to fields in this structure.
 *
 * Methods: tmon_src_create(), tmon_src_delete().
 */
typedef struct tmon_src_s {
  /*! \brief Parent tmon context monitoring object. */
  tmon_ctx_t *tmon_ctx;
  /*! Topic name for application source. */
  char app_topic_str[256];
  /*! Time stamp when this object was created. */
  struct timeval create_time;
  /*! Work buffer for building tmon messages.
   * This is retained to minimize repeated malloc/free. */
  ezstr_t *mon_msg;
} tmon_src_t;


/*! \brief tmon connection monitoring object.
 * Created by tmon_conn_create().
 * Create one per receiver delivery controller from
 * inside the
 * <a href="https://ultramessaging.github.io/currdoc/doc/Config/grpdeliverycontrol.html#sourcenotificationfunctionreceiver">source_notification_function (receiver)</a>
 * callback.
 * That callback can return this object as the source-specific clientd.
 * See \ref clientdatapointers.
 * Application must not write to fields in this structure.
 *
 * Methods: tmon_conn_create(), tmon_conn_delete(), tmon_conn_rcv_event().
 */
typedef struct tmon_conn_s {
  /*! \brief Parent tmon receiver monitoring object. */
  tmon_rcv_t *tmon_rcv;
  /*! Source string for transport session, plus topic index. */
  char source_str[128];
  /*! Time stamp when this object was created. */
  struct timeval create_time;
  /*! Time stamp when <a href="https://ultramessaging.github.io/currdoc/doc/API/lbm_8h.html#ab5489080adc7157549a9930b30c68425">LBM_MSG_BOS</a>
   * receiver event was delivered for this connection
   * (0 if not delivered yet). */
  struct timeval bos_time;
  /*! Time stamp when most-recent <a href="https://ultramessaging.github.io/currdoc/doc/API/lbm_8h.html#a88920e0a4188081f9a14fc8f76c18578">LBM_MSG_UNRECOVERABLE_LOSS</a>
   * or <href="https://ultramessaging.github.io/currdoc/doc/API/lbm_8h.html#a6629139aaf902976c8df9de3f37d10db">LBM_MSG_UNRECOVERABLE_LOSS_BURST</a>
   * receiver event was delivered for this connection
   * (0 if neither delivered yet). */
  struct timeval loss_time;
  /*! Total number of received data messages for this connection. */
  long msg_count;
  /*! Sequence number of last successfully-received message.
   * Used to calculate number of datagrams lost in a burst lost event. */
  long last_sqn;
  /*! Total number of loss events delivered for this connection. */
  long loss_events;
  /*! Total number of lost datagrams (fragments) for this connection. */
  long dgrams_lost;
  /*! Work buffer for building tmon messages.
   * This is retained to minimize repeated malloc/free. */
  ezstr_t *mon_msg;
} tmon_conn_t;


/*! \brief Portable microsecond-precision time stamp.
 *
 * \param tv Pointer to a struct timeval.
 */
void tmon_gettimeofday(struct timeval *tv);

/*! \brief Portable, reentrant converter from Posix "time_t" to ascii date/time.
 *
 * "time_t" is number of seconds since the Unix Epoch (midnight, 1-Jan-1970).
 *
 * \param buffer Byte array at least 27 bytes long.
 *   Date/time string will be written here.
 * \param bufsize Actual number of bytes in "buffer".
 * \param cur_time Posix time value to convert.
 */
const char *tmon_ctime(char *buffer, size_t bufsize, time_t cur_time);

/*! \brief Portable, reentrant converter from binary IPv4 address to ascii.
 *
 * \param addr 32-bit binary form of IPv4 address,
 * as would be present in struct sockaddr_in.sin_addr.s_addr.
 * \param dst Byte array at least 16 bytes long.
 *   Dotted numeric IP address will be written here.
 * \param buf_len Actual size of "dst" buffer.
 * \return pointer to input buffer (returned for convenience).
 */
char *tmon_inet_ntop(lbm_uint_t addr, char *dst, size_t buf_len);

/*! \brief Portable microsecond-precision sleep function.
 *
 * Note that there is no guarantee of accuracy.
 * Many operating systems do not support sleep times under a millisecond.
 * Callers to this function should be prepared to experience much longer
 * sleep times than requested.
 *
 * \param usec number of microseconds to sleep.
 */
void tmon_usleep(int usec);


void tmon_encode_str(char *dst, char *src);

void tmon_decode_str(char *dst, char *src);

/*! \brief Create a tmon context monitoring object,
 * associated with an application UM context.
 * Create one of these per application context immediately
 * *after* creating each context.
 *
 * \param app_ctx Application context.
 * \returns tmon context monitoring object.
 */
tmon_ctx_t *tmon_ctx_create(lbm_context_t *app_ctx);

/*! \brief Delete a tmon_ctx object.
 * Do not delete this object until after all child objects
 * (\ref tmon_rcv_s, \ref tmon_src_s) are deleted.
 *
 * \param tmon_ctx tmon context monitoring object.
 */
void tmon_ctx_delete(tmon_ctx_t *tmon_ctx);


/*! \brief Create a tmon receiver or wildcard receiver monitoring object,
 * associated with an application UM receiver.
 * Create one per application receiver immediately
 * *before* creating each receiver.
 *
 * \param tmon_ctx Parent tmon context monitoring object.
 *   This must be the tmon object associated with the same application
 *   context as the application receiver being created.
 * \param rcv_type Type of UM receiver: \ref TMON_RCV_TYPE_REGULAR, \ref TMON_RCV_TYPE_WILDCARD
 * \param app_topic_str Topic string assoicated with the application
 *   receiver being created.
 * \returns tmon receiver monitoring object.
 */
tmon_rcv_t *tmon_rcv_create(tmon_ctx_t *tmon_ctx, int rcv_type,
  char *app_topic_str);

/*! \brief Delete a tmon receiver monitoring object.
 * Do not delete this object until after all child objects
 * (\ref tmon_conn_s) are deleted.
 *
 * \param tmon_rcv tmon receiver monitoring object.
 */
void tmon_rcv_delete(tmon_rcv_t *tmon_rcv);


/*! \brief Create a tmon source monitoring object,
 * associated with an application UM source.
 * Create one per application source immediately
 * *before* creating each source.
 *
 * \param tmon_ctx Parent context monitoring object.
 *   This must be the tmon object associated with the same application
 *   context as the application source being created.
 * \param app_topic_str Topic string assoicated with the application
 *   source being created.
 * \returns tmon source monitoring object.
 */
tmon_src_t *tmon_src_create(tmon_ctx_t *tmon_ctx, char *app_topic_str);

/*! \brief Delete a tmon source monitoring object.
 *
 * \param tmon_src tmon source monitoring object.
 */
void tmon_src_delete(tmon_src_t *tmon_src);


/*! \brief Create a tmon connection monitoring object to monitor an
 * application UM receiver's
 * <a href="https://ultramessaging.github.io/currdoc/doc/Design/architecture.html#deliverycontroller">delivery controller</a>.
 * A delivery controller can be thought of as "connection" between
 * a source to a topic and a receiver of that same topic.
 * This should be called inside the
 * <a href="https://ultramessaging.github.io/currdoc/doc/Config/grpdeliverycontrol.html#sourcenotificationfunctionreceiver">source_notification_function (receiver)</a>
 * "create" callback.
 * That callback can return this object as the per-source client data.
 * See \ref clientdatapointers.
 *
 * \param tmon_rcv tmon receiver monitoring object.
 *   This must be the tmon object associated with the same application
 *   context as the application source being created.
 * \param source_str source string for this connection, including Topic Index.
 * \returns tmon connection monitoring object.
 *   This should be returned as the per-source client data
 */
tmon_conn_t *tmon_conn_create(tmon_rcv_t *tmon_rcv, const char *source_str);

/*! \brief Delete a tmon connection monitoring object.
 * This should be called inside the
 * <a href="https://ultramessaging.github.io/currdoc/doc/Config/grpdeliverycontrol.html#sourcenotificationfunctionreceiver">source_notification_function (receiver)</a>
 * "delete" callback.
 *
 * \param tmon_conn tmon connection object.
 */
void tmon_conn_delete(tmon_conn_t *tmon_conn);

/*! \brief Record a receiver event on a connection.
 * This should be called inside your receiver callback.
 *
 * \param msg UM message object passed to the receiver callback.
 * \param tmon_conn connection object. This should be available via the
 *   per-source client data in the "msg" structure.
 *   See \ref clientdatapointers.
 */
void tmon_conn_rcv_event(tmon_conn_t *tmon_conn, lbm_msg_t *msg);

/*! \brief Create a UM context based on a standard UM "transport options"
 * string.
 * Called by monitoring tool.
 * The configuration file and topic string are extracted from the
 * transport options and returned to the caller, along with the UM context.
 * This information is subsequently passed to tmon_create_monrcv().
 * Delete this context with the standard UM
 * <a href="https://ultramessaging.github.io/currdoc/doc/API/lbm_8h.html#a962bfceb336c65191ba08497ac70602b">lbm_context_delete()</a> API.
 *
 * \param topic_str Pointer to a byte buffer to be filled with the topic string
 *   taken from the transport options.
 * \param config_file Pointer to a byte buffer to be filled with the name of
 *   the configuration file taken from the transport options.
 * \param buf_lens Size of the topic_str and config_file buffers.
 * \param transport_opts User specified transport options which are passed
 *   to the standard UM monitoring software.
 * \returns UM context.
 */
lbm_context_t *tmon_create_monctx(char *topic_str, char *config_file, int *tmon_loss_suppress, int buf_lens, char *transport_opts);

/*! \brief Create a UM receiver for the tmon data.
 * Delete this receiver with the standard UM
 * <a href="https://ultramessaging.github.io/currdoc/doc/API/lbm_8h.html#a8d5e8713f5ae776330b23a1e371f934d">lbm_rcv_delete()</a> API.
 *
 * \param ctx UM context returned by tmon_create_monctx().
 * \param topic_str UM topic string, returned by tmon_create_monctx().
 * \param config_file UM configuration file, returned by tmon_create_monctx().
 * \param transport_opts User specified transport options which are passed
 *   to the standard UM monitoring software.
 * \param proc User's receiver event callback function.
 * \param clientd Application data that the user wants UM to pass to the
 *   receiver callback.
 * \param evq Optional event queue, if desired, otherwise pass NULL.
 */
lbm_rcv_t *tmon_create_monrcv(lbm_context_t *ctx, const char *topic_str, const char *config_file, const char *transport_opts, lbm_rcv_cb_proc proc, void *clientd, lbm_event_queue_t *evq);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif  /* TMON_H */
