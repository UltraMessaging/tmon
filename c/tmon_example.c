/* tmon_example.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#if defined(_WIN32)
  #pragma warning(disable : 4996)
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <stdint.h>
  #include <sys/timeb.h>
  #define strcasecmp stricmp
  #define strncasecmp strnicmp
  #define sleep(_sec) Sleep((_sec)*1000)
  #define usleep(_usec) do { \
    HANDLE timer; \
    LARGE_INTEGER due_time; \
    due_time.QuadPart = -(10 * (_usec));  \
    timer = CreateWaitableTimer(NULL, TRUE, NULL); \
    SetWaitableTimer(timer, &due_time, 0, NULL, NULL, 0); \
    WaitForSingleObject(timer, INFINITE); \
    CloseHandle(timer);  \
  } while (0)

#else  /* unix */
  #include <unistd.h>
  #include <sys/time.h>
  #include <netdb.h>
#endif

#include <lbm/lbm.h>
#include "tmon.h"


/* Primitive error handling - exit on error, which is rude for a
 *  library function. */
#define NULLCHK(ptr_) do { \
  if ((ptr_) == NULL) { \
    fprintf(stderr, "Null pointer error at %s:%d '%s'\n", \
      __FILE__, __LINE__, #ptr_); \
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


int msgs_rcvd = 0;

int app_rcv_callback(lbm_rcv_t *rcv, lbm_msg_t *msg, void *clientd)
{
  if (msg->source_clientd != NULL) {
    tmon_conn_rcv_event((tmon_conn_t *)msg->source_clientd, msg);
  }

  switch (msg->type)
  {
  case LBM_MSG_DATA:
    printf("app_rcv_callback: LBM_MSG_DATA: %d bytes on topic %s: '%.*s'\n",
           (int)msg->len, msg->topic_name, (int)msg->len, msg->data);

    /* Tell main thread that we've received our message. */
    ++ msgs_rcvd;
    break;

  case LBM_MSG_BOS:
    printf("app_rcv_callback: LBM_MSG_BOS: [%s][%s]\n", msg->topic_name, msg->source);
    break;

  case LBM_MSG_EOS:
    printf("app_rcv_callback: LBM_MSG_EOS: [%s][%s]\n", msg->topic_name, msg->source);

    /* Tell main thread that we've received our message. */
    ++ msgs_rcvd;
    break;

  case LBM_MSG_UNRECOVERABLE_LOSS:
    printf("app_rcv_callback: LBM_MSG_UNRECOVERABLE_LOSS: [%s][%s]\n", msg->topic_name, msg->source);

    /* Tell main thread that we've received our message. */
    ++ msgs_rcvd;
    break;

  case LBM_MSG_UNRECOVERABLE_LOSS_BURST:
    printf("app_rcv_callback: LBM_MSG_UNRECOVERABLE_LOSS_BURST: [%s][%s]\n", msg->topic_name, msg->source);

    /* Tell main thread that we've received our message. */
    ++ msgs_rcvd;
    break;

  default:    /* unexpected receiver event */
    printf("app_rcv_callback: default: [%s][%s], type=%d\n", msg->topic_name, msg->source, msg->type);
    break;
  }  /* switch msg->type */

  return 0;
}  /* app_rcv_callback */


void *dc_create_cb(const char *source_name, void *clientd)
{
  tmon_rcv_t *tmon_rcv = (tmon_rcv_t *)clientd;

  fprintf(stdout, "dc_create_cb: source_name='%s', topic='%s'\n", source_name, tmon_rcv->app_topic_str);

  return tmon_conn_create(tmon_rcv, source_name);
}  /* dc_create_cb */


int dc_delete_cb(const char *source_name, void *clientd, void *source_clientd)
{
  tmon_conn_t *tmon_conn = (tmon_conn_t *)source_clientd;

  fprintf(stdout, "dc_delete_cb: source_name='%s'\n", source_name);

  tmon_conn_delete(tmon_conn);

  return 0;
}  /* dc_delete_cb */


int main(int argc, char **argv)
{
  char *cfg_file;
  char *test;
  /* Context */
  lbm_context_attr_t *ctx_attr;
  lbm_context_t *ctx;
  lbm_topic_t *topic_obj;
  /* Receiver */
  lbm_rcv_src_notification_func_t src_notif_func;
  lbm_rcv_topic_attr_t *rcv_attr;
  lbm_rcv_t *rcv1;
  lbm_rcv_t *rcv3;
  lbm_rcv_t *rcvx;
  /* Wildcard Receiver */
  lbm_wildcard_rcv_t *wrcv;
  /* Source */
  lbm_src_topic_attr_t *src_attr;
  lbm_src_t *src1;
  lbm_src_t *src2;
  lbm_src_t *src3;
  /* Other */
  tmon_ctx_t *tmon_ctx;
  tmon_rcv_t *tmon_rcv1;
  tmon_rcv_t *tmon_rcv3;
  tmon_rcv_t *tmon_rcvx;
  tmon_rcv_t *tmon_wrcv;
  tmon_src_t *tmon_src1;
  tmon_src_t *tmon_src2;
  tmon_src_t *tmon_src3;

#if defined(_WIN32)
  {
    WSADATA wsadata;
    int status;

    /* Windows socket setup code */
    if ((status = WSAStartup(MAKEWORD(2,2),&wsadata)) != 0) {
      fprintf(stderr,"%s: WSA startup error - %d\n",argv[0],status);
      exit(1);
    }
  }
#endif

  if (argc == 3) {
    cfg_file = argv[1];  /* Application config file. */
    test = argv[2];
  } else if (argc == 2) {
    cfg_file = argv[1];
    test = "1";
  } else if (argc == 1) {
    cfg_file = "application.cfg";
    test = "1";
  } else {
    fprintf(stderr, "Usage tmon_example [config_file [test_num]]\n");
    exit(1);
  }
  LBMCHK(lbm_config(cfg_file));  /* Application config file. */

  printf("Creating context.\n");
  LBMCHK(lbm_context_attr_create_from_xml(&ctx_attr, "tmon_example_ctx"));
  LBMCHK(lbm_context_create(&ctx, ctx_attr, NULL, NULL));
  LBMCHK(lbm_context_attr_delete(ctx_attr));

  printf("Creating topic monitor.\n");
  tmon_ctx = tmon_ctx_create(ctx);

  printf("Creating wildcard rcv for '^.*2$' (will resolv, no msg)\n");
  src_notif_func.clientd = tmon_wrcv = tmon_rcv_create(tmon_ctx, TMON_RCV_TYPE_WILDCARD, "^.*2$");
  LBMCHK(lbm_rcv_topic_attr_create_from_xml(&rcv_attr, "tmon_example_ctx", "wc2"));
  src_notif_func.create_func = dc_create_cb;
  src_notif_func.delete_func = dc_delete_cb;
  LBMCHK(lbm_rcv_topic_attr_setopt(rcv_attr, "source_notification_function", &src_notif_func, sizeof(src_notif_func)));
  LBMCHK(lbm_wildcard_rcv_create(&wrcv, ctx, "^.*2$", rcv_attr, NULL, app_rcv_callback, tmon_wrcv, NULL));
  LBMCHK(lbm_rcv_topic_attr_delete(rcv_attr));

  printf("Creating rcv for 'src1' (will resolv, rcv msg)\n");
  src_notif_func.clientd = tmon_rcv1 = tmon_rcv_create(tmon_ctx, TMON_RCV_TYPE_REGULAR, "src1");
  LBMCHK(lbm_rcv_topic_attr_create_from_xml(&rcv_attr, "tmon_example_ctx", "src1"));
  src_notif_func.create_func = dc_create_cb;
  src_notif_func.delete_func = dc_delete_cb;
  LBMCHK(lbm_rcv_topic_attr_setopt(rcv_attr, "source_notification_function", &src_notif_func, sizeof(src_notif_func)));
  LBMCHK(lbm_rcv_topic_lookup(&topic_obj, ctx, "src1", rcv_attr));
  LBMCHK(lbm_rcv_create(&rcv1, ctx, topic_obj, app_rcv_callback, tmon_rcv1, NULL));
  LBMCHK(lbm_rcv_topic_attr_delete(rcv_attr));

  printf("Creating rcv for 'src3' (will resolve, no msg)\n");
  LBMCHK(lbm_rcv_topic_attr_create_from_xml(&rcv_attr, "tmon_example_ctx", "src2"));
  src_notif_func.create_func = dc_create_cb;
  src_notif_func.delete_func = dc_delete_cb;
  src_notif_func.clientd = tmon_rcv3 = tmon_rcv_create(tmon_ctx, TMON_RCV_TYPE_REGULAR, "src3");
  LBMCHK(lbm_rcv_topic_attr_setopt(rcv_attr, "source_notification_function", &src_notif_func, sizeof(src_notif_func)));
  LBMCHK(lbm_rcv_topic_lookup(&topic_obj, ctx, "src3", rcv_attr));
  LBMCHK(lbm_rcv_create(&rcv3, ctx, topic_obj, app_rcv_callback, tmon_rcv3, NULL));
  LBMCHK(lbm_rcv_topic_attr_delete(rcv_attr));

  printf("Creating rcv for 'srcx' (will not resolve)\n");
  LBMCHK(lbm_rcv_topic_attr_create_from_xml(&rcv_attr, "tmon_example_ctx", "srcx"));
  src_notif_func.create_func = dc_create_cb;
  src_notif_func.delete_func = dc_delete_cb;
  src_notif_func.clientd = tmon_rcvx = tmon_rcv_create(tmon_ctx, TMON_RCV_TYPE_REGULAR, "srcx");
  LBMCHK(lbm_rcv_topic_attr_setopt(rcv_attr, "source_notification_function", &src_notif_func, sizeof(src_notif_func)));
  LBMCHK(lbm_rcv_topic_lookup(&topic_obj, ctx, "srcx", rcv_attr));
  LBMCHK(lbm_rcv_create(&rcvx, ctx, topic_obj, app_rcv_callback, tmon_rcvx, NULL));
  LBMCHK(lbm_rcv_topic_attr_delete(rcv_attr));

  printf("Creating src for 'src1' (will resolve, send msg)\n");
  tmon_src1 = tmon_src_create(tmon_ctx, "src1");
  LBMCHK(lbm_src_topic_attr_create_from_xml(&src_attr, "tmon_example_ctx", "src1"));
  LBMCHK(lbm_src_topic_alloc(&topic_obj, ctx, "src1", src_attr));
  LBMCHK(lbm_src_create(&src1, ctx, topic_obj, NULL, NULL, NULL));
  LBMCHK(lbm_src_topic_attr_delete(src_attr));

  printf("Creating src for 'src2' (wildcard resolve, no msg)\n");
  tmon_src2 = tmon_src_create(tmon_ctx, "src2");
  LBMCHK(lbm_src_topic_attr_create_from_xml(&src_attr, "tmon_example_ctx", "src2"));
  LBMCHK(lbm_src_topic_alloc(&topic_obj, ctx, "src2", src_attr));
  LBMCHK(lbm_src_create(&src2, ctx, topic_obj, NULL, NULL, NULL));
  LBMCHK(lbm_src_topic_attr_delete(src_attr));

  printf("Creating src for 'src3' (will resolve, no msg)\n");
  tmon_src3 = tmon_src_create(tmon_ctx, "src3");
  LBMCHK(lbm_src_topic_attr_create_from_xml(&src_attr, "tmon_example_ctx", "src3"));
  LBMCHK(lbm_src_topic_alloc(&topic_obj, ctx, "src3", src_attr));
  LBMCHK(lbm_src_create(&src3, ctx, topic_obj, NULL, NULL, NULL));
  LBMCHK(lbm_src_topic_attr_delete(src_attr));

  printf("Sleeping 1 for TR - should get DC creates for 'src1' and 'src2'.\n");
  sleep(1);
  printf("Sleeping 6 for BOS triggered by TSNI for 'src1' and 'src2'\n");
  sleep(6);

  printf("Deleting rcv for 'src3' while src still up - should get DC delete without EOS\n");
  LBMCHK(lbm_rcv_delete(rcv3));
  tmon_rcv_delete(tmon_rcv3);

  if (strcmp(test, "1") == 0) {
    printf("Sending to 'src1'\n");
    LBMCHK(lbm_src_send(src1, "Hello!", 6, LBM_MSG_FLUSH));

    printf("Wait for msg receive or EOS\n");
    while (msgs_rcvd == 0) {
      sleep(1);
    }
  }
  else if (strcmp(test, "2") == 0) {
    int i;
    printf("Sending 120 msgs to 'src1'\n");
    /* 10 msgs/sec for 12 sec. */
    for (i = 0; i < 120; i++) {
      LBMCHK(lbm_src_send(src1, "Hello!", 6, LBM_MSG_FLUSH));
      usleep(100000);
    }
    printf("lingering 10 sec\n");
    sleep(10);
  }
  else {
    printf("bad test number: '%s'\n", test);
  }

  printf("Delete sources\n");
  LBMCHK(lbm_src_delete(src1));
  tmon_src_delete(tmon_src1);
  LBMCHK(lbm_src_delete(src2));
  tmon_src_delete(tmon_src2);
  LBMCHK(lbm_src_delete(src3));
  tmon_src_delete(tmon_src3);

  printf("Sleeping 6 sec for EOS and DC delete for 'src1'.\n");
  sleep(6);

  printf("Deleting rcvs for 'src1', 'srcx'\n");
  LBMCHK(lbm_rcv_delete(rcv1));
  tmon_rcv_delete(tmon_rcv1);
  LBMCHK(lbm_rcv_delete(rcvx));
  tmon_rcv_delete(tmon_rcvx);
  LBMCHK(lbm_wildcard_rcv_delete(wrcv));
  tmon_rcv_delete(tmon_wrcv);

  printf("Deleting tmon_ctx.\n");
  tmon_ctx_delete(tmon_ctx);

  printf("Deleting context.\n");
  LBMCHK(lbm_context_delete(ctx));

  return 0;
}  /* main */
