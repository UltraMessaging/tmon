<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>tmon.h</name>
    <path>/Users/sford/Documents/GitHub/tmon/c/</path>
    <filename>tmon_8h</filename>
    <class kind="struct">ezstr_s</class>
    <class kind="struct">tmon_ctx_s</class>
    <class kind="struct">tmon_rcv_s</class>
    <class kind="struct">tmon_src_s</class>
    <class kind="struct">tmon_conn_s</class>
    <member kind="define">
      <type>#define</type>
      <name>TMON_STR_BUF_LENS</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a7ca2d2973383b11c628a04169a3d1667</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct ezstr_s</type>
      <name>ezstr_t</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>aa0008486a47e54986e3928729ed42a23</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct tmon_ctx_s</type>
      <name>tmon_ctx_t</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>abf674a8c3a416e5135f9b343b0886c8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct tmon_rcv_s</type>
      <name>tmon_rcv_t</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a3fd849f02596f1e422356f063b57228c</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct tmon_src_s</type>
      <name>tmon_src_t</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>ab71817970e75cdd4fa5bc6ab9a18992f</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct tmon_conn_s</type>
      <name>tmon_conn_t</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a49c87df5e1bc16d86b92e06000299aab</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>tmon_gettimeofday</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>afb4497c2c37e85282d33e482f9b8e327</anchor>
      <arglist>(struct timeval *tv)</arglist>
    </member>
    <member kind="function">
      <type>const char *</type>
      <name>tmon_ctime</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>ad55643acc71c2e2b1815806a6dcb2aec</anchor>
      <arglist>(char *buffer, size_t bufsize, time_t cur_time)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>tmon_inet_ntop</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>aa565cea893fe35b05f38ccb37085b1e2</anchor>
      <arglist>(lbm_uint_t addr, char *dst, size_t buf_len)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>tmon_usleep</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a426744b9073362dd0ecda7433fc1e488</anchor>
      <arglist>(int usec)</arglist>
    </member>
    <member kind="function">
      <type>tmon_ctx_t *</type>
      <name>tmon_ctx_create</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>aed61a720a2e795ea899af4642edad0a6</anchor>
      <arglist>(lbm_context_t *app_ctx)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>tmon_ctx_delete</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>ab40619ac905b3b0f3f9c06d399e020ee</anchor>
      <arglist>(tmon_ctx_t *tmon_ctx)</arglist>
    </member>
    <member kind="function">
      <type>tmon_rcv_t *</type>
      <name>tmon_rcv_create</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a5602a423bea1a50ae9890a6cd5090307</anchor>
      <arglist>(tmon_ctx_t *tmon_ctx, int rcv_type, char *app_topic_str)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>tmon_rcv_delete</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>aa0afa8049a793ad1a3560e20c7670c99</anchor>
      <arglist>(tmon_rcv_t *tmon_rcv)</arglist>
    </member>
    <member kind="function">
      <type>tmon_src_t *</type>
      <name>tmon_src_create</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a7a05fb72b16dad515a5ebf0efd295edf</anchor>
      <arglist>(tmon_ctx_t *tmon_ctx, char *app_topic_str)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>tmon_src_delete</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>aa3d416665f06258cd9c0c759a2a8b271</anchor>
      <arglist>(tmon_src_t *tmon_src)</arglist>
    </member>
    <member kind="function">
      <type>tmon_conn_t *</type>
      <name>tmon_conn_create</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a1bcd235ecc04d3dbf8c61713ad1a3ac7</anchor>
      <arglist>(tmon_rcv_t *tmon_rcv, const char *source_str)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>tmon_conn_delete</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a90c529737495dadd30030db2f32f71ab</anchor>
      <arglist>(tmon_conn_t *tmon_conn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>tmon_conn_rcv_event</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>afbb5b7f5b09fd4e961c1b4944d11ea7f</anchor>
      <arglist>(lbm_msg_t *msg, tmon_conn_t *tmon_conn)</arglist>
    </member>
    <member kind="function">
      <type>lbm_context_t *</type>
      <name>tmon_create_context</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>ab09d9c9ed73960a763b4fdd1b55fbc35</anchor>
      <arglist>(char *topic_str, char *config_file, int buf_lens, char *transport_opts)</arglist>
    </member>
    <member kind="function">
      <type>lbm_rcv_t *</type>
      <name>tmon_create_monrcv</name>
      <anchorfile>tmon_8h.html</anchorfile>
      <anchor>a1040837000444f89cd161168df0f872e</anchor>
      <arglist>(lbm_context_t *ctx, const char *topic_str, const char *config_file, const char *transport_opts, lbm_rcv_cb_proc proc, void *clientd, lbm_event_queue_t *evq)</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>ezstr_s</name>
    <filename>structezstr__s.html</filename>
    <member kind="variable">
      <type>char *</type>
      <name>buff</name>
      <anchorfile>structezstr__s.html</anchorfile>
      <anchor>a99600147d393376018a7cbcc2e401a88</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>size_t</type>
      <name>alloc_size</name>
      <anchorfile>structezstr__s.html</anchorfile>
      <anchor>a792932ce184623ef534d676969e63bcb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>size_t</type>
      <name>null_index</name>
      <anchorfile>structezstr__s.html</anchorfile>
      <anchor>a2113f194816bdf96447ccb9d0b60e79b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tmon_conn_s</name>
    <filename>structtmon__conn__s.html</filename>
    <member kind="variable">
      <type>tmon_rcv_t *</type>
      <name>tmon_rcv</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>a3dbe0b52bba4466eb48dcca11b6063d3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>source_str</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>a6d288b5de800b578ca5e8aea67a26aab</anchor>
      <arglist>[128]</arglist>
    </member>
    <member kind="variable">
      <type>struct timeval</type>
      <name>create_time</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>a7bc80fe8d1d57b6e80defdbf69e32424</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct timeval</type>
      <name>bos_time</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>acd7e13765167815686b24c8241e62d18</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct timeval</type>
      <name>loss_time</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>a016211953b392b2a8a374709d2ecf589</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>long</type>
      <name>msg_count</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>a5f3f274ddacb569930e889a32dc19b1b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>long</type>
      <name>unrec_count</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>af9042361a825a1b53cd1fa57bfec23f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>long</type>
      <name>burst_count</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>ac9a53fc0fbd166fa8acf49c8ff1efa15</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ezstr_t *</type>
      <name>mon_msg</name>
      <anchorfile>structtmon__conn__s.html</anchorfile>
      <anchor>aa8ae3c7cb73784cf0a84f0d0cb393654</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tmon_ctx_s</name>
    <filename>structtmon__ctx__s.html</filename>
    <member kind="variable">
      <type>lbm_context_t *</type>
      <name>app_ctx</name>
      <anchorfile>structtmon__ctx__s.html</anchorfile>
      <anchor>a7f57aac1846340728146b4e0cd3f728c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>transport_opts</name>
      <anchorfile>structtmon__ctx__s.html</anchorfile>
      <anchor>a499b85c63a78984cf4bdce5ba3751f4a</anchor>
      <arglist>[2048]</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>app_id</name>
      <anchorfile>structtmon__ctx__s.html</anchorfile>
      <anchor>a18c9cfdc754a243a45315b9cf6f9ecc1</anchor>
      <arglist>[TMON_STR_BUF_LENS]</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>config_file</name>
      <anchorfile>structtmon__ctx__s.html</anchorfile>
      <anchor>aacef6aa4ad1599aa0b9e34d14d363069</anchor>
      <arglist>[TMON_STR_BUF_LENS]</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>topic_str</name>
      <anchorfile>structtmon__ctx__s.html</anchorfile>
      <anchor>ae8dfa6d6954bc960d6c3084548373e8f</anchor>
      <arglist>[TMON_STR_BUF_LENS]</arglist>
    </member>
    <member kind="variable">
      <type>lbm_context_t *</type>
      <name>ctx</name>
      <anchorfile>structtmon__ctx__s.html</anchorfile>
      <anchor>ac1c391ae7d763f69454157aa9ebf3750</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>lbm_src_t *</type>
      <name>src</name>
      <anchorfile>structtmon__ctx__s.html</anchorfile>
      <anchor>ae90a8d0c6f2a45fe34bb678d76d98658</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ezstr_t *</type>
      <name>header</name>
      <anchorfile>structtmon__ctx__s.html</anchorfile>
      <anchor>a804cab3a967136f9004104e9f5858206</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tmon_rcv_s</name>
    <filename>structtmon__rcv__s.html</filename>
    <member kind="variable">
      <type>tmon_ctx_t *</type>
      <name>tmon_ctx</name>
      <anchorfile>structtmon__rcv__s.html</anchorfile>
      <anchor>a8e72d87430e972b564bf64c1b15581d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>rcv_type</name>
      <anchorfile>structtmon__rcv__s.html</anchorfile>
      <anchor>a7feaa3f22dcf7bacb226d02a88a22ee0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>app_topic_str</name>
      <anchorfile>structtmon__rcv__s.html</anchorfile>
      <anchor>a063382e63c718dfda9fbb39b09e61a4a</anchor>
      <arglist>[TMON_STR_BUF_LENS]</arglist>
    </member>
    <member kind="variable">
      <type>struct timeval</type>
      <name>create_time</name>
      <anchorfile>structtmon__rcv__s.html</anchorfile>
      <anchor>ab08e5027f00b5ed028b2b87a7ac02797</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ezstr_t *</type>
      <name>mon_msg</name>
      <anchorfile>structtmon__rcv__s.html</anchorfile>
      <anchor>a97b46e77cfe9bb24cb8daae511b2af10</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tmon_src_s</name>
    <filename>structtmon__src__s.html</filename>
    <member kind="variable">
      <type>tmon_ctx_t *</type>
      <name>tmon_ctx</name>
      <anchorfile>structtmon__src__s.html</anchorfile>
      <anchor>a5987dcae40b2237685d7116f83a758e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>app_topic_str</name>
      <anchorfile>structtmon__src__s.html</anchorfile>
      <anchor>ab35a18c4d06bf6b80e825517d287cb8e</anchor>
      <arglist>[256]</arglist>
    </member>
    <member kind="variable">
      <type>struct timeval</type>
      <name>create_time</name>
      <anchorfile>structtmon__src__s.html</anchorfile>
      <anchor>ac6150d55482eea1601803323dfe11371</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>ezstr_t *</type>
      <name>mon_msg</name>
      <anchorfile>structtmon__src__s.html</anchorfile>
      <anchor>a37d179685e3fb5ce85dd93379b4d1e47</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>tmon v0.1 UM Topic Monitor API</title>
    <filename>index</filename>
    <docanchor file="index" title="C API Overview">capioverview</docanchor>
  </compound>
</tagfile>
