/* tmon.c - UM Topic Monitor
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#if defined(_WIN32)
  #pragma warning(disable : 4996)
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <stdint.h>
  #include <sys/timeb.h>
  #define strcasecmp stricmp
  #define strncasecmp strnicmp
  #define sleep(_sec) Sleep((_sec)*1000)

#else  /* unix */
  #include <unistd.h>
  #include <sys/time.h>
  #include <netdb.h>
  #include <arpa/inet.h>
#endif

#include <lbm/lbm.h>
#include <lbm/lbmaux.h>
#include "tmon.h"


/* Primitive error handling - exit on error, which is rude for a
 * library function. */
#define NULLCHK(ptr_) do { \
  if ((ptr_) == NULL) { \
    fprintf(stderr, "Null pointer error at %s:%d '%s'\n", \
      __FILE__, __LINE__, #ptr_); \
    fflush(stderr); \
    exit(1); \
  } \
} while (0)
#define ASSRT(cond_) do { \
  if (! (cond_)) { \
    fprintf(stderr, "Failed assert at %s:%d '%s'\n", \
      __FILE__, __LINE__, #cond_); \
    fflush(stderr); \
    exit(1); \
  } \
} while (0)
#define FAILCHK(status_) do { \
  if ((status_) == -1) { \
    perror("Failed status"); \
    fprintf(stderr, "Failure at %s:%d '%s'\n", \
      __FILE__, __LINE__, #status_); \
    fflush(stderr); \
    exit(1); \
  } \
} while (0)
#define LBMCHK(status_) do { \
  if ((status_) == LBM_FAILURE) { \
    fprintf(stderr, "LBM Failure at %s:%d '%s', '%s'\n", \
      __FILE__, __LINE__, #status_, lbm_errmsg()); \
    fflush(stderr); \
    exit(1); \
  } \
} while (0)


/* Portable usleep() */
void tmon_usleep(int usec)
{
#if defined(_WIN32)
  HANDLE timer;
  LARGE_INTEGER due_time;
  due_time.QuadPart = -(10 * (usec));
  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  SetWaitableTimer(timer, &due_time, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
#else
  usleep(usec);
#endif
}  /* tmon_usleep */


/* Portable gettimeofday(). */
void tmon_gettimeofday(struct timeval *tv)
{
#if defined(_WIN32)
  FILETIME file_time;
  uint64_t time;

  GetSystemTimeAsFileTime(&file_time);
  time = (uint64_t)file_time.dwLowDateTime
       + (((uint64_t)file_time.dwHighDateTime) << 32);

  /* Windows uses Jan 1, 1601 epoc. Unix uses Jan 1, 1970. */
  tv->tv_sec = (long)((time - 116444736000000000ULL) / 10000000ULL);
  tv->tv_usec = (long)((time % 10000000ULL) / 10ULL);
#else
  gettimeofday(tv, NULL);
#endif
}  /* tmon_gettimeofday */


/* Portable ctime, without the trailing newline. */
const char *tmon_ctime(char *buffer, size_t bufsize, time_t cur_time)
{
  size_t len;
#if defined(_WIN32)
  errno_t err = ctime_s(buffer, bufsize, &cur_time);
  ASSRT(err == 0);
#else 
  char *res = ctime_r(&cur_time, buffer);
  NULLCHK(res);
#endif

  len = strlen(buffer);
  if (len > 0 && buffer[len-1] == '\n') {
    buffer[len-1] = '\0';
  }

  return buffer;
}  /* tmon_ctime */


/* Reentrantly and portably convert binary net address to dotted string. */
char *tmon_inet_ntop(lbm_uint_t addr, char *dst, size_t buf_len)
{
#if defined(_WIN32)
  struct sockaddr_in sockaddr;
  long rtn_val = 0;

  memset(&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = addr;
  rtn_val = WSAAddressToString((LPSOCKADDR)&sockaddr, sizeof(sockaddr), NULL, dst, &buf_len);
  if (rtn_val != 0) {
    strcpy(dst, "Error");
  }
#else
  struct in_addr iaddr;

  iaddr.s_addr = addr;
  if (inet_ntop(AF_INET, &iaddr, dst, buf_len) == NULL) {
    strcpy(dst, "Error");
  }
#endif
  return dst;
}  /* tmon_inet_ntop */


/*
 * Small string class to simplify the code.
 */

static ezstr_t *ezstr_create()
{
  ezstr_t *ezstr = (ezstr_t *)malloc(sizeof(ezstr_t));
  NULLCHK(ezstr);

  ezstr->alloc_size = 1024;
  ezstr->buff = (char *)malloc(ezstr->alloc_size);
  NULLCHK(ezstr->buff);

  ezstr->null_index = 0;
  ezstr->buff[ezstr->null_index] = '\0';

  return ezstr;
}  /* ezstr_create */

static void ezstr_append(ezstr_t *ezstr, char *str)
{
  size_t str_len = strlen(str);

  if ((ezstr->null_index + str_len) >= ezstr->alloc_size) {
    /* Allocation too small, make it double the needed size. */
    ezstr->buff = (char *)realloc(ezstr->buff,
      2 * (ezstr->null_index + str_len));
    NULLCHK(ezstr->buff);
  }

  memcpy(&ezstr->buff[ezstr->null_index], str, str_len + 1);
  ezstr->null_index += str_len;
}  /* ezstr_append */

static void ezstr_clear(ezstr_t *ezstr)
{
  ezstr->null_index = 0;
  ezstr->buff[ezstr->null_index] = '\0';
}  /* ezstr_clear */

static void ezstr_delete(ezstr_t *ezstr)
{
  free(ezstr->buff);
  free(ezstr);
}  /* ezstr_delete */


/* Encode string to hide commas. Does not work "in-place". */
void tmon_encode_str(char *dst, char *src)
{
  while (*src != '\0') {
    if (*src == '\\') {
      *dst = '\\';
      dst++;
      *dst = '1';
      dst++;
    }
    else if (*src == ',') {
      *dst = '\\';
      dst++;
      *dst = '2';
      dst++;
    }
    else {
      *dst = *src;
      dst++;
    }
    src++;
  }  /* while */
}  /* tmon_encode_str */

/* Decode string to retrieve commas. Can be used "in-place". */
void tmon_decode_str(char *dst, char *src)
{
  while (*src != '\0') {
    if (*src == '\\') {
      if (*(src+1) == '1') {
        *dst = '\\';
        dst++;
        src += 2;
      }
      else if (*(src+1) == '2') {
        *dst = ',';
        dst++;
        src += 2;
      }
      else {
        *dst = *src;
        dst++;
        src++;
      }
    }
    else {
      *dst = *src;
      dst++;
      src++;
    }
  }  /* while */

  *dst = '\0';
}  /* tmon_decode_str */


/*
 * Functions to parse monitoring config options (taken from lbmmon).
 */

/* Return 0 for valid, -1 for not valid */
static int scope_is_valid(const char * Scope)
{
  int valid = -1;

  if (strcasecmp(Scope, "context") == 0 ||
      strcasecmp(Scope, "source") == 0 ||
      strcasecmp(Scope, "receiver") == 0 ||
      strcasecmp(Scope, "event_queue") == 0) {
    valid = 0;
  }
  return valid;
}  /* scope_is_valid */


/*
 * Parse transport_opts, which consists of semicolon-separated key=val
 * pairs.
 */
static const char *tmon_next_pair(const char *transport_opts,
  char *key, size_t key_size, char *val, size_t val_size)
{
  const char *str_ptr = transport_opts;
  size_t pos;

  if ((str_ptr == NULL) || (*str_ptr == '\0') || (key == NULL)
    || (key_size == 0) || (val == NULL) || (val_size == 0)) {
    return (NULL);
  }

  memset(key, 0, key_size);
  memset(val, 0, val_size);

  /* Process the key */
  pos = 0;
  /* Skip leading whitespace */
  while (((*str_ptr == ' ') || (*str_ptr == '\t')) && (*str_ptr != '\0')) {
    str_ptr++;
  }
  /* Gather key until reach end of string, an equal sign (delimiting the */
  /* value), a semicolon (delimiting the pair), or we fill key. */
  while ((*str_ptr != '\0') && (*str_ptr != '=') && (*str_ptr != ';')
      && (pos < (key_size - 1))) {
    key[pos++] = *str_ptr++;
  }
  /* Skip everything until the equal sign, semicolon, or end of string. */
  while ((*str_ptr != '\0') && (*str_ptr != '=') && (*str_ptr != ';')) {
    str_ptr++;
  }

  /* What to do next? */
  if (*str_ptr == '\0') {
    /* No = or value. Leave pointing to NUL for next call. */
    return (str_ptr);
  }
  else if (*str_ptr == ';') {
    /* No = or value. Point to character beyond semicolon for next call. */
    return (++str_ptr);
  }

  /* val follows. Advance str_ptr to next character. */
  str_ptr++;

  /* Process the value */
  pos = 0;
  /* Skip leading whitespace */
  while (((*str_ptr == ' ') || (*str_ptr == '\t')) && (*str_ptr != '\0')) {
    str_ptr++;
  }
  /* Gather value until we reach end of string, a semicolon (delimiting */
  /* the pair), or we fill val. */
  while ((*str_ptr != '\0') && (*str_ptr != ';') && (pos < (val_size - 1))) {
    val[pos++] = *str_ptr++;
  }
  /* Skip everything until the semicolon or end of string. */
  while ((*str_ptr != '\0') && (*str_ptr != ';')) {
    str_ptr++;
  }
  if (*str_ptr == ';') {
    str_ptr++;
  }

  return (str_ptr);
}  /* tmon_next_pair */


static lbm_uint_t get_host_address_net()
{
  char buf[256];
  int rc;
  struct hostent *hostptr;
  lbm_uint_t address;

  rc = gethostname(buf, sizeof(buf));
  if (rc == 0) {
    hostptr = gethostbyname(buf);
    if (hostptr != NULL) {
      address = *(lbm_uint_t *)(*hostptr->h_addr_list);
    }
    else {
      address = (lbm_uint_t)inet_addr(buf);
    }
  }
  else {
    address = 0;
  }

  return address;
}  /* get_host_address_net */


static lbm_ulong_t get_process_id()
{
#if defined(_WIN32)
  return (lbm_ulong_t) GetCurrentProcessId();
#else
  return (lbm_ulong_t) getpid();
#endif
}  /* get_process_id */


static void tmon_get_settings(tmon_ctx_t *tmon_ctx)
{
  size_t opt_len;

  opt_len = sizeof(tmon_ctx->app_id);
  LBMCHK(lbm_context_getopt(tmon_ctx->app_ctx, "monitor_appid", tmon_ctx->app_id,
    &opt_len));

  opt_len = sizeof(tmon_ctx->transport_opts);
  LBMCHK(lbm_context_getopt(tmon_ctx->app_ctx, "monitor_transport_opts",
    tmon_ctx->transport_opts, &opt_len));
}  /* tmon_get_settings */


/* Give each Tmon object a unique ID using atomic increment. */
static long current_object_count = 0;

void tmon_get_next_object_id()
{
  return tmon_inline_atomic_long_incr(&current_object_count);
}  /* tmon_get_next_object_id */


/* Parse transport_opts string and create context.
 * Called by monitoring tool.
 * Pass back topic string and config file, and return context. */
lbm_context_t *tmon_create_monctx(char *topic_str, char *config_file,
  int *tmon_loss_suppress, int buf_lens, char *transport_opts)
{
  lbm_context_attr_t *ctx_attr;
  const char *opt_p;
  char key[512];
  char value[512];
  lbm_context_t *ctx;

  if (topic_str != NULL) {
    strncpy(topic_str, "/29west/tmon", buf_lens);  /* default */
  }
  if (tmon_loss_suppress != NULL) {
    *tmon_loss_suppress = 1;  /* default */
  }
  if (config_file != NULL) {
    strncpy(config_file, "", buf_lens);  /* default */
  }

  /* Step through options in monitor_transport_opts to find topic and config. */
  opt_p = transport_opts;
  while ((opt_p = tmon_next_pair(opt_p, key, sizeof(key), value, sizeof(value))) != NULL) {
    if (strcasecmp(key, "tmon_topic") == 0) {
      if (topic_str != NULL) {
        strncpy(topic_str, value, buf_lens);
      }
    }
    else if (strcasecmp(key, "tmon_loss_suppress") == 0) {
      if (tmon_loss_suppress != NULL) {
        *tmon_loss_suppress = atoi(value);
      }
    }
    else if (strcasecmp(key, "config") == 0) {
      if (config_file != NULL) {
        strncpy(config_file, value, buf_lens);
      }
    }
  }  /* while opt_p */

  /* Initialize context attributes. */
  LBMCHK(lbm_context_attr_create_from_xml(&ctx_attr, "29west_tmon_context"));

  if (config_file[0] != '\0') {
    /* Read context attribs from config file. */
    LBMCHK(lbmaux_context_attr_setopt_from_file(ctx_attr, config_file));
  }

  /* Apply any context options supplied in the monitor_transport_opts.
   * These override the config file. */
  opt_p = transport_opts;
  while ((opt_p = tmon_next_pair(opt_p, key, sizeof(key), value, sizeof(value))) != NULL) {
    char scope[512], option[512];
    if (sscanf(key, "%[a-zA-Z_]|%[a-zA-Z_]", scope, option) != 2) {
      continue;  /* Not a config option. */
    }
    FAILCHK(scope_is_valid(scope));
    if (strcasecmp(scope, "context") == 0) {
      LBMCHK(lbm_context_attr_str_setopt(ctx_attr, option, value));
    }
  }  /* while opt_p */

  /* Create context. */
  LBMCHK(lbm_context_create(&ctx, ctx_attr, NULL, NULL));
  LBMCHK(lbm_context_attr_delete(ctx_attr));

  return ctx;
}  /* tmon_create_monctx */


static void tmon_create_monsrc(tmon_ctx_t *tmon_ctx)
{
  lbm_src_topic_attr_t *src_attr;
  lbm_topic_t *topic_obj;
  const char *opt_p;
  char key[512];
  char value[512];

  /* Initialize source attributes. */
  LBMCHK(lbm_src_topic_attr_create_from_xml(&src_attr, "29west_tmon_context", tmon_ctx->topic_str));

  if (tmon_ctx->config_file[0] != '\0') {
    /* Read source attribs from config file. */
    LBMCHK(lbmaux_src_topic_attr_setopt_from_file(src_attr, tmon_ctx->config_file));
  }

  /* Apply any source options supplied in the monitor_transport_opts.
   * These override the config file. */
  opt_p = tmon_ctx->transport_opts;
  while ((opt_p = tmon_next_pair(opt_p, key, sizeof(key), value, sizeof(value))) != NULL) {
    char scope[512], option[512];
    if (sscanf(key, "%[a-zA-Z_]|%[a-zA-Z_]", scope, option) != 2) {
      continue;  /* Not a config option. */
    }
    FAILCHK(scope_is_valid(scope));
    if (strcasecmp(scope, "source") == 0) {
      LBMCHK(lbm_src_topic_attr_str_setopt(src_attr, option, value));
    }
  }  /* while opt_p */

  /* Create source. */
  LBMCHK(lbm_src_topic_alloc(&topic_obj, tmon_ctx->ctx, tmon_ctx->topic_str, src_attr));
  LBMCHK(lbm_src_create(&tmon_ctx->src, tmon_ctx->ctx, topic_obj, NULL, NULL, NULL));
  LBMCHK(lbm_src_topic_attr_delete(src_attr));
}  /* tmon_create_monsrc */


lbm_rcv_t *tmon_create_monrcv(lbm_context_t *ctx, const char *topic_str,
  const char *config_file, const char *transport_opts,
  lbm_rcv_cb_proc proc, void *clientd, lbm_event_queue_t *evq)
{
  lbm_rcv_t *rcv;
  lbm_rcv_topic_attr_t *rcv_attr;
  lbm_topic_t *topic_obj;
  const char *opt_p;
  char key[512];
  char value[512];

  /* Initialize source attributes. */
  LBMCHK(lbm_rcv_topic_attr_create_default(&rcv_attr));

  if (config_file[0] != '\0') {
    /* Read source attribs from config file. */
    LBMCHK(lbmaux_rcv_topic_attr_setopt_from_file(rcv_attr, config_file));
  }

  /* Apply any receiver options supplied in the monitor_transport_opts.
   * These override the config file. */
  opt_p = transport_opts;
  while ((opt_p = tmon_next_pair(opt_p, key, sizeof(key), value, sizeof(value))) != NULL) {
    char scope[512], option[512];
    if (sscanf(key, "%[a-zA-Z_]|%[a-zA-Z_]", scope, option) != 2) {
      continue;  /* Not a config option. */
    }
    FAILCHK(scope_is_valid(scope));
    if (strcasecmp(scope, "receiver") == 0) {
      LBMCHK(lbm_rcv_topic_attr_str_setopt(rcv_attr, option, value));
    }
  }  /* while opt_p */

  /* Create receiver. */
  LBMCHK(lbm_rcv_topic_lookup(&topic_obj, ctx, topic_str, rcv_attr));
  LBMCHK(lbm_rcv_create(&rcv, ctx, topic_obj, proc, clientd, evq));
  LBMCHK(lbm_rcv_topic_attr_delete(rcv_attr));

  return rcv;
}  /* tmon_create_monrcv */


static void tmon_init_header(tmon_ctx_t *tmon_ctx)
{
  char work_str[2*TMON_STR_BUF_LENS+1];

  /* app_id, IP, PID, app_ctx */

  ezstr_append(tmon_ctx->header, tmon_ctx->app_id);
  ezstr_append(tmon_ctx->header, ",");

  ezstr_append(tmon_ctx->header,
    tmon_inet_ntop(get_host_address_net(), work_str, sizeof(work_str)));
  ezstr_append(tmon_ctx->header, ",");

  sprintf(work_str, "%lu,", get_process_id());
  ezstr_append(tmon_ctx->header, work_str);

  sprintf(work_str, "%lu", (unsigned long)tmon_ctx->app_ctx);
  ezstr_append(tmon_ctx->header, work_str);
}  /* tmon_init_header */


/*
 * Topic Monitor object implementation.
 */

tmon_ctx_t *tmon_ctx_create(lbm_context_t *app_ctx)
{
  tmon_ctx_t *tmon_ctx;

  tmon_ctx = (tmon_ctx_t *)malloc(sizeof(tmon_ctx_t));
  NULLCHK(tmon_ctx);

  tmon_ctx->header = ezstr_create();
  NULLCHK(tmon_ctx->header);

  tmon_ctx->app_ctx = app_ctx;

  /* Get settings from monitoring config options.  */
  tmon_get_settings(tmon_ctx);

  /* Create tmon context with same characteristics as UM monitoring context.  */
  tmon_ctx->ctx = tmon_create_monctx(tmon_ctx->topic_str, tmon_ctx->config_file,
    &tmon_ctx->tmon_loss_suppress, TMON_STR_BUF_LENS, tmon_ctx->transport_opts);

  /* Create tmon source with same characteristics as UM monitoring source.  */
  tmon_create_monsrc(tmon_ctx);

  /* Create tmon message header string.  */
  tmon_init_header(tmon_ctx);

  return tmon_ctx;
}  /* tmon_ctx_create */


void tmon_ctx_delete(tmon_ctx_t *tmon_ctx)
{
  LBMCHK(lbm_src_flush(tmon_ctx->src));  /* flush out batching queue. */
  LBMCHK(lbm_src_delete(tmon_ctx->src));
  LBMCHK(lbm_context_delete(tmon_ctx->ctx));
  ezstr_delete(tmon_ctx->header);
  free(tmon_ctx);
}  /* tmon_ctx_delete */


/* Call this right before creating a UM receiver object. */
tmon_rcv_t *tmon_rcv_create(tmon_ctx_t *tmon_ctx, int rcv_type,
  char *app_topic_str)
{
  tmon_rcv_t *tmon_rcv;
  char work_str[2*TMON_STR_BUF_LENS+1];

  tmon_rcv = (tmon_rcv_t *)malloc(sizeof(tmon_rcv_t));
  NULLCHK(tmon_rcv);

  tmon_rcv->rcv_type = rcv_type;
  tmon_rcv->mon_msg = ezstr_create();
  NULLCHK(tmon_rcv->mon_msg);
  tmon_rcv->tmon_ctx = tmon_ctx;
  strncpy(tmon_rcv->app_topic_str, app_topic_str,
    sizeof(tmon_rcv->app_topic_str));
  tmon_gettimeofday(&tmon_rcv->create_time);

  /* Prepare monitoring message. */
  ezstr_clear(tmon_rcv->mon_msg);
  if (tmon_rcv->rcv_type == TMON_RCV_TYPE_REGULAR) {
    ezstr_append(tmon_rcv->mon_msg, "R,");
  }
  else if (tmon_rcv->rcv_type == TMON_RCV_TYPE_WILDCARD) {
    ezstr_append(tmon_rcv->mon_msg, "W,");
  }
  else ASSRT(tmon_rcv->rcv_type == TMON_RCV_TYPE_REGULAR || tmon_rcv->rcv_type == TMON_RCV_TYPE_WILDCARD);

  ezstr_append(tmon_rcv->mon_msg, tmon_ctx->header->buff);
  sprintf(work_str, ",%ld,%ld,%lu,%s",
    tmon_rcv->create_time.tv_sec, (long)tmon_rcv->create_time.tv_usec,
    (unsigned long)tmon_rcv, tmon_rcv->app_topic_str);
  ezstr_append(tmon_rcv->mon_msg, work_str);

  /* Send it. */
  LBMCHK(lbm_src_send(tmon_ctx->src, tmon_rcv->mon_msg->buff,
    tmon_rcv->mon_msg->null_index, 0));

  return tmon_rcv;
}  /* tmon_rcv_create */


/* Call this right after deleting a UM receiver object. */
void tmon_rcv_delete(tmon_rcv_t *tmon_rcv)
{
  tmon_ctx_t *tmon_ctx;
  struct timeval delete_time;
  char work_str[2*TMON_STR_BUF_LENS+1];

  tmon_ctx = tmon_rcv->tmon_ctx;
  NULLCHK(tmon_ctx);

  tmon_gettimeofday(&delete_time);

  /* Prepare monitoring message. */
  ezstr_clear(tmon_rcv->mon_msg);
  if (tmon_rcv->rcv_type == TMON_RCV_TYPE_REGULAR) {
    ezstr_append(tmon_rcv->mon_msg, "r,");
  }
  else if (tmon_rcv->rcv_type == TMON_RCV_TYPE_WILDCARD) {
    ezstr_append(tmon_rcv->mon_msg, "w,");
  }
  else ASSRT(tmon_rcv->rcv_type == TMON_RCV_TYPE_REGULAR || tmon_rcv->rcv_type == TMON_RCV_TYPE_WILDCARD);
  ezstr_append(tmon_rcv->mon_msg, tmon_ctx->header->buff);
  sprintf(work_str, ",%ld,%ld,%lu",
    delete_time.tv_sec, (long)delete_time.tv_usec, (unsigned long)tmon_rcv);
  ezstr_append(tmon_rcv->mon_msg, work_str);

  /* Send it. */
  LBMCHK(lbm_src_send(tmon_ctx->src, tmon_rcv->mon_msg->buff,
    tmon_rcv->mon_msg->null_index, 0));

  ezstr_delete(tmon_rcv->mon_msg);

  free(tmon_rcv);
}  /* tmon_rcv_delete */


tmon_src_t *tmon_src_create(tmon_ctx_t *tmon_ctx, char *app_topic_str)
{
  tmon_src_t *tmon_src;
  char work_str[2*TMON_STR_BUF_LENS+1];

  tmon_src = (tmon_src_t *)malloc(sizeof(tmon_src_t));
  NULLCHK(tmon_src);

  tmon_src->mon_msg = ezstr_create();
  NULLCHK(tmon_src->mon_msg);
  tmon_src->tmon_ctx = tmon_ctx;
  strncpy(tmon_src->app_topic_str, app_topic_str,
    sizeof(tmon_src->app_topic_str));
  tmon_gettimeofday(&tmon_src->create_time);

  /* Prepare monitoring message. */
  ezstr_clear(tmon_src->mon_msg);
  ezstr_append(tmon_src->mon_msg, "S,");
  ezstr_append(tmon_src->mon_msg, tmon_ctx->header->buff);
  sprintf(work_str, ",%ld,%ld,%lu,%s",
    tmon_src->create_time.tv_sec, (long)tmon_src->create_time.tv_usec,
    (unsigned long)tmon_src, tmon_src->app_topic_str);
  ezstr_append(tmon_src->mon_msg, work_str);

  /* Send it. */
  LBMCHK(lbm_src_send(tmon_ctx->src, tmon_src->mon_msg->buff,
    tmon_src->mon_msg->null_index, 0));

  return tmon_src;
}  /* tmon_src_create */


void tmon_src_delete(tmon_src_t *tmon_src)
{
  tmon_ctx_t *tmon_ctx;
  struct timeval delete_time;
  char work_str[2*TMON_STR_BUF_LENS+1];

  tmon_ctx = tmon_src->tmon_ctx;
  NULLCHK(tmon_ctx);

  tmon_gettimeofday(&delete_time);

  /* Prepare monitoring message. */
  ezstr_clear(tmon_src->mon_msg);
  ezstr_append(tmon_src->mon_msg, "s,");
  ezstr_append(tmon_src->mon_msg, tmon_ctx->header->buff);
  sprintf(work_str, ",%ld,%ld,%lu",
    delete_time.tv_sec, (long)delete_time.tv_usec, (unsigned long)tmon_src);
  ezstr_append(tmon_src->mon_msg, work_str);

  /* Send it. */
  LBMCHK(lbm_src_send(tmon_ctx->src, tmon_src->mon_msg->buff,
    tmon_src->mon_msg->null_index, 0));

  ezstr_delete(tmon_src->mon_msg);

  free(tmon_src);
}  /* tmon_src_delete */


tmon_conn_t *tmon_conn_create(tmon_rcv_t *tmon_rcv, const char *source_str)
{
  tmon_conn_t *tmon_conn;
  tmon_ctx_t *tmon_ctx;
  char work_str[2*TMON_STR_BUF_LENS+1];

  NULLCHK(tmon_rcv);
  tmon_ctx = tmon_rcv->tmon_ctx;
  NULLCHK(tmon_ctx);

  tmon_conn = (tmon_conn_t *)malloc(sizeof(tmon_conn_t));
  NULLCHK(tmon_conn);

  tmon_conn->mon_msg = ezstr_create();
  NULLCHK(tmon_conn->mon_msg);
  tmon_conn->tmon_rcv = tmon_rcv;
  strncpy(tmon_conn->source_str, source_str,
    sizeof(tmon_conn->source_str));
  tmon_gettimeofday(&tmon_conn->create_time);
  tmon_conn->bos_time.tv_sec = 0;
  tmon_conn->bos_time.tv_usec = 0;
  tmon_conn->loss_time.tv_sec = 0;
  tmon_conn->loss_time.tv_usec = 0;
  tmon_conn->msg_count = 0;
  tmon_conn->last_sqn = 0;
  tmon_conn->loss_events = 0;
  tmon_conn->dgrams_lost = 0;

  /* Prepare monitoring message. */
  ezstr_clear(tmon_conn->mon_msg);
  ezstr_append(tmon_conn->mon_msg, "C,");
  ezstr_append(tmon_conn->mon_msg, tmon_ctx->header->buff);
  sprintf(work_str, ",%ld,%ld,%lu,%lu,%s",
    tmon_conn->create_time.tv_sec, (long)tmon_conn->create_time.tv_usec,
    (unsigned long)tmon_conn, (unsigned long)tmon_rcv, tmon_conn->source_str);
  ezstr_append(tmon_conn->mon_msg, work_str);

  /* Send it. */
  LBMCHK(lbm_src_send(tmon_ctx->src, tmon_conn->mon_msg->buff,
    tmon_conn->mon_msg->null_index, 0));

  return tmon_conn;
}  /* tmon_conn_create */


void tmon_conn_delete(tmon_conn_t *tmon_conn)
{
  tmon_rcv_t *tmon_rcv;
  tmon_ctx_t *tmon_ctx;
  struct timeval delete_time;
  char work_str[2*TMON_STR_BUF_LENS+1];

  NULLCHK(tmon_conn);
  tmon_rcv = tmon_conn->tmon_rcv;
  NULLCHK(tmon_rcv);
  tmon_ctx = tmon_rcv->tmon_ctx;
  NULLCHK(tmon_ctx);

  tmon_gettimeofday(&delete_time);

  /* Prepare monitoring message. */
  ezstr_clear(tmon_conn->mon_msg);
  ezstr_append(tmon_conn->mon_msg, "c,");
  ezstr_append(tmon_conn->mon_msg, tmon_ctx->header->buff);
  sprintf(work_str, ",%ld,%ld,%lu,%ld,%ld,%ld",
    delete_time.tv_sec, (long)delete_time.tv_usec, (unsigned long)tmon_conn,
    tmon_conn->msg_count, tmon_conn->loss_events, tmon_conn->dgrams_lost);
  ezstr_append(tmon_conn->mon_msg, work_str);

  /* Send it. */
  LBMCHK(lbm_src_send(tmon_ctx->src, tmon_conn->mon_msg->buff,
    tmon_conn->mon_msg->null_index, 0));

  ezstr_delete(tmon_conn->mon_msg);

  free(tmon_conn);
}  /* tmon_conn_delete */


void tmon_conn_bos(tmon_conn_t *tmon_conn, lbm_msg_t *msg)
{
  tmon_rcv_t *tmon_rcv;
  tmon_ctx_t *tmon_ctx;
  struct timeval event_time;
  char work_str[2*TMON_STR_BUF_LENS+1];

  tmon_rcv = tmon_conn->tmon_rcv;
  NULLCHK(tmon_rcv);
  tmon_ctx = tmon_rcv->tmon_ctx;
  NULLCHK(tmon_ctx);

  tmon_gettimeofday(&event_time);
  ezstr_clear(tmon_conn->mon_msg);
  ezstr_append(tmon_conn->mon_msg, "B,");
  ezstr_append(tmon_conn->mon_msg, tmon_ctx->header->buff);
  sprintf(work_str, ",%ld,%ld,%lu,%s",
    event_time.tv_sec, (long)event_time.tv_usec, (unsigned long)tmon_conn,
    msg->topic_name);
  ezstr_append(tmon_conn->mon_msg, work_str);

  /* Send it. */
  LBMCHK(lbm_src_send(tmon_ctx->src, tmon_conn->mon_msg->buff,
    tmon_conn->mon_msg->null_index, 0));

  /* Remember that BOS happened. */
  tmon_conn->bos_time = event_time;
}  /* tmon_conn_bos */


void tmon_conn_eos(tmon_conn_t *tmon_conn, lbm_msg_t *msg)
{
  tmon_rcv_t *tmon_rcv;
  tmon_ctx_t *tmon_ctx;
  struct timeval event_time;
  char work_str[2*TMON_STR_BUF_LENS+1];

  tmon_rcv = tmon_conn->tmon_rcv;
  NULLCHK(tmon_rcv);
  tmon_ctx = tmon_rcv->tmon_ctx;
  NULLCHK(tmon_ctx);

  tmon_gettimeofday(&event_time);
  ezstr_clear(tmon_conn->mon_msg);
  ezstr_append(tmon_conn->mon_msg, "E,");
  ezstr_append(tmon_conn->mon_msg, tmon_ctx->header->buff);
  sprintf(work_str, ",%ld,%ld,%lu,%ld,%ld,%ld,%s",
    event_time.tv_sec, (long)event_time.tv_usec, (unsigned long)tmon_conn,
    tmon_conn->msg_count, tmon_conn->dgrams_lost, tmon_conn->loss_events,
    msg->topic_name);
  ezstr_append(tmon_conn->mon_msg, work_str);

  /* Send it. */
  LBMCHK(lbm_src_send(tmon_ctx->src, tmon_conn->mon_msg->buff,
    tmon_conn->mon_msg->null_index, 0));

  if (tmon_conn->bos_time.tv_sec == 0) {
    /* Alert! EOS without BOS. */
    ezstr_clear(tmon_conn->mon_msg);
    ezstr_append(tmon_conn->mon_msg, "A,");
    ezstr_append(tmon_conn->mon_msg, tmon_ctx->header->buff);
    sprintf(work_str, ",%ld,%ld,",
      event_time.tv_sec, (long)event_time.tv_usec);
    ezstr_append(tmon_conn->mon_msg, work_str);

    sprintf(work_str, "EOS without BOS, topic='%s', source_str='%s'",
      tmon_rcv->app_topic_str, tmon_conn->source_str);
    ezstr_append(tmon_conn->mon_msg, work_str);

    /* Send it. */
    LBMCHK(lbm_src_send(tmon_ctx->src, tmon_conn->mon_msg->buff,
      tmon_conn->mon_msg->null_index, 0));
  }
}  /* tmon_conn_eos */


void tmon_conn_loss(tmon_conn_t *tmon_conn, lbm_msg_t *msg)
{
  tmon_rcv_t *tmon_rcv;
  tmon_ctx_t *tmon_ctx;
  struct timeval event_time;
  char work_str[2*TMON_STR_BUF_LENS+1];

  tmon_conn->loss_events ++;
  tmon_conn->dgrams_lost ++;

  tmon_rcv = tmon_conn->tmon_rcv;
  NULLCHK(tmon_rcv);
  tmon_ctx = tmon_rcv->tmon_ctx;
  NULLCHK(tmon_ctx);

  tmon_gettimeofday(&event_time);
  if (event_time.tv_sec - tmon_conn->loss_time.tv_sec <= 10) {
    /* Already sent a unrec monitoring message during this 10-second period. */
  }
  else {
    tmon_conn->loss_time = event_time;
    ezstr_clear(tmon_conn->mon_msg);
    ezstr_append(tmon_conn->mon_msg, "L,");
    ezstr_append(tmon_conn->mon_msg, tmon_ctx->header->buff);
    sprintf(work_str, ",%ld,%ld,%ld,%ld",
      event_time.tv_sec, (long)event_time.tv_usec, (unsigned long)tmon_conn,
      (unsigned long)msg->sequence_number);
    ezstr_append(tmon_conn->mon_msg, work_str);

    /* Send it. */
    LBMCHK(lbm_src_send(tmon_ctx->src, tmon_conn->mon_msg->buff,
      tmon_conn->mon_msg->null_index, 0));
  }
  tmon_conn->last_sqn = msg->sequence_number;
}  /* tmon_conn_loss */


void tmon_conn_burst_loss(tmon_conn_t *tmon_conn, lbm_msg_t *msg)
{
  tmon_rcv_t *tmon_rcv;
  tmon_ctx_t *tmon_ctx;
  struct timeval event_time;
  char work_str[2*TMON_STR_BUF_LENS+1];

  tmon_conn->loss_events ++;
  tmon_conn->dgrams_lost += (msg->sequence_number - tmon_conn->last_sqn);

  tmon_rcv = tmon_conn->tmon_rcv;
  NULLCHK(tmon_rcv);
  tmon_ctx = tmon_rcv->tmon_ctx;
  NULLCHK(tmon_ctx);

  tmon_gettimeofday(&event_time);
  if (event_time.tv_sec - tmon_conn->loss_time.tv_sec <= 10) {
    /* Already sent a unrec monitoring message during this 10-second period. */
  }
  else {
    tmon_conn->loss_time = event_time;
    ezstr_clear(tmon_conn->mon_msg);
    ezstr_append(tmon_conn->mon_msg, "l,");
    ezstr_append(tmon_conn->mon_msg, tmon_ctx->header->buff);
    sprintf(work_str, ",%ld,%ld,%lu,%ld,%ld",
      event_time.tv_sec, (long)event_time.tv_usec, (unsigned long)tmon_conn,
      (unsigned long)msg->sequence_number, tmon_conn->last_sqn);
    ezstr_append(tmon_conn->mon_msg, work_str);

    /* Send it. */
    LBMCHK(lbm_src_send(tmon_ctx->src, tmon_conn->mon_msg->buff,
      tmon_conn->mon_msg->null_index, 0));
  }
  tmon_conn->last_sqn = msg->sequence_number;
}  /* tmon_conn_loss */


void tmon_conn_rcv_event(tmon_conn_t *tmon_conn, lbm_msg_t *msg)
{
  NULLCHK(tmon_conn);

  switch (msg->type) {
  case LBM_MSG_DATA:
    tmon_conn->msg_count++;
    tmon_conn->last_sqn = msg->sequence_number;
    break;

  case LBM_MSG_BOS:
    tmon_conn_bos(tmon_conn, msg);
    break;

  case LBM_MSG_EOS:
    tmon_conn_eos(tmon_conn, msg);
    break;

  case LBM_MSG_UNRECOVERABLE_LOSS:
    tmon_conn_loss(tmon_conn, msg);
    break;

  case LBM_MSG_UNRECOVERABLE_LOSS_BURST:
    tmon_conn_burst_loss(tmon_conn, msg);
    break;
  }  /* switch */
}  /* tmon_conn_rcv_event */
