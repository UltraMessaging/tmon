# tmon v0.1 README - Topic Monitor for Ultra Messaging

## REPOSITORY

See https://github.com/UltraMessaging/tmon for code and documentation.

## DOCUMENTATION

[https://ultramessaging.github.io/tmon/html/manual/](https://ultramessaging.github.io/tmon/html/manual/)
- on-line, rendered, "bleeding-edge" version of doc.

Protocol Description

Each message is a single C-style null-terminated string.

S - src_create : S,app_id,IP,PID,ctx,tmon_src,tv_sec,tv_usec,topic
s - src_delete : s,app_id,IP,PID,ctx,tmon_src,tv_sec,tv_usec
R - rcv_create : R,app_id,IP,PID,ctx,tmon_rcv,tv_sec,tv_usec,topic
r - rcv_delete : r,app_id,IP,PID,ctx,tmon_rcv,tv_sec,tv_usec
C - conn_create: C,app_id,IP,PID,ctx,tmon_rcv,conn,tv_sec,tv_usec,src_str
c - conn_delete: c,app_id,IP,PID,ctx,conn,tv_sec,tv_usec,msg_count,unrec_count,burst_count
B - BOS        : b,app_id,IP,PID,ctx,conn,tv_sec,tv_usec
E - EOS        : e,app_id,IP,PID,ctx,conn,tv_sec,tv_usec,msg_count,unrec_count,burst_count
A - Alarm      : A,app_id,IP,PID,ctx,tv_sec,tv_usec,,err_str
  - currently, eos without bos. In future: rcv w/o conn timeout, conn w/o bos timeout.
L - loss       : L,app_id,IP,PID,ctx,conn,tv_sec,tv_usec,msg_count,unrec_count,burst_count
  - probably should be done via timer. Initially just limit to 1/sec, with tv referring to first instance.
