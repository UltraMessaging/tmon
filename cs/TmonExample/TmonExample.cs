/*
  Copyright (c) 2020 Informatica Corporation  Permission is granted to licensees to use
  or alter this software for any purpose, including commercial applications,
  according to the terms laid out in the Software License Agreement.

  This source code example is provided by Informatica for educational
  and evaluation purposes only.

  THE SOFTWARE IS PROVIDED "AS IS" AND INFORMATICA DISCLAIMS ALL WARRANTIES 
  EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF 
  NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
  PURPOSE.  INFORMATICA DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE 
  UNINTERRUPTED OR ERROR-FREE.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES,
  BE LIABLE TO LICENSEE FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR 
  INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGREEMENT OR THE 
  TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BEEN APPRISED OF 
  THE LIKELIHOOD OF SUCH DAMAGES.
*/

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using com.latencybusters.lbm;
using com.latencybusters.Tmon;

namespace TmonExample
{
  class TmonExample
  {
    private int _messagesReceived;
    TmonGetTimeOfDay _timeOfDay = null;
    private readonly object _logLock = new object();

    void Log(String s) {
      lock (_logLock) {
        _timeOfDay.NewTimeStamp();
        string oline = String.Format("{0}.{1,6:000000}: {2}",
            _timeOfDay.GetSec(),
            _timeOfDay.GetUsec(),
            s);
        Console.WriteLine(oline);
      }
    }

    void Run(string[] args) {
      string cfgFile = "application.cfg";
      string test = "1";
      if (args.Length == 2) {
        cfgFile = args[0];
        test = args[1];
      }
      else if (args.Length == 1) {
        cfgFile = args[0];
      }
      else if (args.Length != 0) {
        throw new TmonException("TmonExample: bad arg count ("+args.Length+")");
      }

      LBM lbm = new LBM();
      lbm.setLogger(this.Logger);

      _timeOfDay = new TmonGetTimeOfDay();

      LBM.setConfiguration(cfgFile);

      Log("Creating context.");
      // Context name must be alpha, numeric, underscore, hyphen.
      LBMContextAttributes ctxAttr = new LBMContextAttributes("tmon_example_ctx");
      LBMContext ctx = new LBMContext(ctxAttr);

      Log("Creating topic monitor.");
      TmonContext tmonContext = new TmonContext(ctx);
      tmonContext.InitSender();

      Log("Creating wildcard rcv for '^.*2$' (will resolv, no msg)");
      TmonReceiver tmonWRcv = tmonContext.ReceiverCreate(ReceiverType.Wildcard, "^.*2$");
      LBMReceiverAttributes rcvAttr = new LBMReceiverAttributes("29west_tmon_context", "wc2");
      rcvAttr.setSourceNotificationCallbacks(this.onDeliveryControllerCreate,
          this.onDeliveryControllerDelete, tmonWRcv);
      LBMWildcardReceiver wrcv = new LBMWildcardReceiver(ctx, "^.*2$", rcvAttr,
          null, this.OnReceive, null);

      Log("Creating rcv for 'src\\1' (will resolv, rcv msg)");
      TmonReceiver tmonRcv1 = tmonContext.ReceiverCreate(ReceiverType.Regular, "src\\1");
      rcvAttr = new LBMReceiverAttributes("29west_tmon_context", "src\\1");
      rcvAttr.setSourceNotificationCallbacks(this.onDeliveryControllerCreate,
          this.onDeliveryControllerDelete, tmonRcv1);
      LBMReceiver rcv1 = new LBMReceiver(ctx, ctx.lookupTopic("src\\1", rcvAttr),
          this.OnReceive, null, null);

      Log("Creating rcv for 'src3' (will resolve, no msg)");
      TmonReceiver tmonRcv3 = tmonContext.ReceiverCreate(ReceiverType.Regular, "src3");
      rcvAttr = new LBMReceiverAttributes("29west_tmon_context", "src3");
      rcvAttr.setSourceNotificationCallbacks(this.onDeliveryControllerCreate,
          this.onDeliveryControllerDelete, tmonRcv3);
      LBMReceiver rcv3 = new LBMReceiver(ctx, ctx.lookupTopic("src3", rcvAttr),
          this.OnReceive, null, null);

      Log("Creating rcv for 'srcx' (will not resolve)");
      TmonReceiver tmonRcvx = tmonContext.ReceiverCreate(ReceiverType.Regular, "srcx");
      rcvAttr = new LBMReceiverAttributes("29west_tmon_context", "srcx");
      rcvAttr.setSourceNotificationCallbacks(this.onDeliveryControllerCreate,
          this.onDeliveryControllerDelete, tmonRcvx);
      LBMReceiver rcvx = new LBMReceiver(ctx, ctx.lookupTopic("srcx", rcvAttr),
          this.OnReceive, null, null);

      Log("Creating src for 'src\\1' (will resolve, send msg)");
      TmonSource tmonSrc1 = tmonContext.SourceCreate("src\\1");
      LBMSourceAttributes srcAttr = new LBMSourceAttributes("29west_tmon_context", "src\\1");
      LBMSource src1 = ctx.createSource(ctx.allocTopic("src\\1", srcAttr));
      Thread.Sleep(100);  // Let receiver discover.

      Log("Creating src for 'src,2' (wildcard resolve, no msg)");
      TmonSource tmonSrc2 = tmonContext.SourceCreate("src,2");
      srcAttr = new LBMSourceAttributes("29west_tmon_context", "src,2");
      LBMSource src2 = ctx.createSource(ctx.allocTopic("src,2", srcAttr));
      Thread.Sleep(100);  // Let receiver discover.

      Log("Creating src for 'src3' (will resolve, no msg)");
      TmonSource tmonSrc3 = tmonContext.SourceCreate("src3");
      srcAttr = new LBMSourceAttributes("29west_tmon_context", "src3");
      LBMSource src3 = ctx.createSource(ctx.allocTopic("src3", srcAttr));
      Thread.Sleep(100);  // Let receiver discover.

      Log("Delete receiver for 'src,2' to get DC delete without BOS or EOS");
      wrcv.close();
      tmonWRcv.Close();

      Log("Sleeping 6 for BOS triggered by TSNI for 'src\\1', 'src3'");
      Thread.Sleep(6000);

      Log("Deleting rcv for 'src3' while src still up - should get DC delete without EOS");
      rcv3.close();
      tmonRcv3.Close();

      string msg = "Hello!";
      if (test.Equals("1")) {
        Log("Sending to 'src\\1'");
        src1.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);

        Log("Wait for msg receive or EOS");
        while (_messagesReceived == 0) {
          Thread.Sleep(1000);
        }
      }
      else if (test.Equals("2")) {
        Log("Sending 120 msgs to 'src\\1'");
        for (int i = 0; i < 120; i++) {
          src1.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);
        }  // for
        Log("lingering 10 sec");
        Thread.Sleep(10000);
      }
      else {
        throw new TmonException("TmonExample: bad test number ("+test+")");
      }

      Log("Delete sources");
      src1.close();
      tmonSrc1.Close();
      src2.close();
      tmonSrc2.Close();
      src3.close();
      tmonSrc3.Close();

      Log("Sleeping 6 sec for EOS and DC delete for 'src\\1'.");
      Thread.Sleep(6000);

      Log("Deleting rcvs for 'src\\1'");
      rcv1.close();
      tmonRcv1.Close();

      Log("Generate rolling EOS on srcx");
      LBM.setLbtrmSrcLossRate(100);
      TmonSource tmonSrcx = tmonContext.SourceCreate("srcx");
      srcAttr = new LBMSourceAttributes("29west_tmon_context", "srcx");
      LBMSource srcx = ctx.createSource(ctx.allocTopic("srcx", srcAttr));
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Start SMs going.
      Thread.Sleep(13750);  // Wait 2.5 * RM activity timeouts.

      Log("Generate BOS on srcx and msg");
      LBM.setLbtrmSrcLossRate(0);
      msg = "1";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Trigger BOS.
      Thread.Sleep(100);

      Log("Generate unrecoverable loss on srcx");
      LBM.setLbtrmSrcLossRate(100);
      msg = "2";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Lost pkt.
      Thread.Sleep(100);
      LBM.setLbtrmSrcLossRate(0);
      msg = "3";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // gap.
      Thread.Sleep(200);  // 2*nak gen ivl.
      msg = "4";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // push out loss event.
      Thread.Sleep(100);

      Log("Generate suppressed unrecoverable loss");
      LBM.setLbtrmSrcLossRate(0);
      msg = "1";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Trigger BOS.
      Thread.Sleep(100);
      LBM.setLbtrmSrcLossRate(100);
      msg = "2";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Lost pkt.
      Thread.Sleep(100);
      LBM.setLbtrmSrcLossRate(0);
      msg = "3";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // gap.
      Thread.Sleep(200);  // 2*nak gen ivl.
      msg = "4";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // push out loss event.
      Thread.Sleep(100);

      Thread.Sleep(10100);  // Expire tmon's 10-second loss suppression.

      Log("Generate unrecoverable burst loss");
      LBM.setLbtrmSrcLossRate(100);
      msg = "5";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Lost pkt.
      msg = "6";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Lost pkt.
      msg = "7";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Lost pkt.
      msg = "8";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // Lost pkt.
      Thread.Sleep(100);
      LBM.setLbtrmSrcLossRate(0);
      msg = "9";
      srcx.send(Encoding.ASCII.GetBytes(msg), msg.Length, 0);  // gap.
      Thread.Sleep(200);  // Wait for BURST.

      Log("Delete rcvx, srcx.");
      rcvx.close();
      tmonRcvx.Close();
      srcx.close();
      tmonSrcx.Close();

      Log("Deleting tmon_ctx.");
      tmonContext.Close();

      Log("Deleting context.");
      ctx.close();
    }  // Run

    static void Main(string[] args) {
      TmonExample example = new TmonExample();
      try {
        example.Run(args);
      } catch (Exception e) {
        example.Log("Exception: " + e.Message);
        Console.WriteLine(e);
      }
    }  // Main


    private void Logger(int logLevel, string message) {
      switch (logLevel) {
        case LBM.LOG_ALERT: Log("Alert: " + message); break;
        case LBM.LOG_CRIT: Log("Critical: " + message); break;
        case LBM.LOG_DEBUG: Log("Debug: " + message); break;
        case LBM.LOG_EMERG: Log("Emergency: " + message); break;
        case LBM.LOG_ERR: Log("Error: " + message); break;
        case LBM.LOG_INFO: Log("Info: " + message); break;
        case LBM.LOG_NOTICE: Log("Note: " + message); break;
        case LBM.LOG_WARNING: Log("Warning: " + message); break;
        default: Log("Unknown: " + message); break;
      }
    }  // Logger


    public int OnReceive(object cbArg, LBMMessage msg) {
      TmonConnection conn = (TmonConnection)msg.sourceClientObject();
      if (conn != null) {
        conn.ReceiverEvent(msg);
      }

      switch (msg.type()) {
        case LBM.MSG_DATA:
          Log("app_rcv_callback: LBM_MSG_DATA: "
              + (long)msg.length() + " bytes on topic " + msg.topicName() + ":"
              + msg.dataString());

          _messagesReceived++;
          break;

        case LBM.MSG_BOS:
          Log("app_rcv_callback: LBM_MSG_BOS: ["
              + msg.topicName() + "][" + msg.source() + "]");
          break;

        case LBM.MSG_EOS:
          Log("app_rcv_callback: LBM_MSG_EOS: ["
              + msg.topicName() + "][" + msg.source() + "]");
          _messagesReceived++;
          break;

        case LBM.MSG_UNRECOVERABLE_LOSS:
          Log("app_rcv_callback: LBM_MSG_UNRECOVERABLE_LOSS: ["
              + msg.topicName() + "][" + msg.source() + "]");
          _messagesReceived++;
          break;

        case LBM.MSG_UNRECOVERABLE_LOSS_BURST:
          Log("app_rcv_callback: LBM_MSG_UNRECOVERABLE_LOSS_BURST: ["
              + msg.topicName() + "][" + msg.source() + "]");
          _messagesReceived++;
          break;

        default:  // Unexpected receiver event.
          Log("app_rcv_callback: default: ["
              + msg.topicName() + "][" + msg.source() + "], type=" + msg.type());
          break;
      }  // switch

      return 0;
    }  // OnReceive


    public object onDeliveryControllerCreate(string sourceName, object cbArg) {
      TmonReceiver tmonRcv = (TmonReceiver) cbArg;

      Log("dc_create_cb: source_name='" + sourceName + "'");

      return tmonRcv.ConnectionCreate(sourceName);
    }  // onDeliveryControllerCreate

    public int onDeliveryControllerDelete(string sourceName,
        object cbArg, object sourceCbArg) {
      TmonConnection tmonConn = (TmonConnection)sourceCbArg;

      Log("dc_delete_cb: source_name='" + sourceName);

      tmonConn.Close();
      return 0;
    }  // onDeliveryControllerDelete

  }  // TmonExample
}  // TMONExample
