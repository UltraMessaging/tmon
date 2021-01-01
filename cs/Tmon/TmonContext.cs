/*
  Copyright (c) 2020 Informatica Corporation
  Permission is granted to licensees to use
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
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;  
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using com.latencybusters.lbm;

namespace com.latencybusters.Tmon {
#if (ANYCPU == false)
#if X64
  using size_t = System.Int64;
#else
  using size_t = System.Int32;
#endif
#endif

  public class TmonContext : IDisposable {
    // The following native C function from the UM library is
    // not technically a public API. Therefore, it doesn't have
    // a CS wrapper. However, it is accessible, and using it
    // ensures a uniform behavior in config file parsing.
    [DllImport(LBM.LBM_SHARE)]
#if (ANYCPU == false)
    private static unsafe extern void lbm_parse_config_line(
        [MarshalAs(UnmanagedType.LPStr)] string configLine,
        [MarshalAs(UnmanagedType.LPStr)] StringBuilder scope,
        [MarshalAs(UnmanagedType.LPStr)] StringBuilder option,
        [MarshalAs(UnmanagedType.LPStr)] StringBuilder value,
        size_t *matches,
        size_t *confLength);
#else
    private static unsafe extern void lbm_parse_config_line(
        [MarshalAs(UnmanagedType.LPStr)] string configLine,
        [MarshalAs(UnmanagedType.LPStr)] StringBuilder scope,
        [MarshalAs(UnmanagedType.LPStr)] StringBuilder option,
        [MarshalAs(UnmanagedType.LPStr)] StringBuilder value,
        IntPtr *matches,
        IntPtr *confLength);
#endif

    // Static member for generating unique object IDs.
    private static int _currentObjectCount = 0;

    private int _objectId = 0;
    private LBMContext _appUmContext = null;
    private string _appId = null;
    private string _transportOptsStr = null;
    private string[] _transportOpts;
    private string _appContextName = null;
	private string _cfgFile = null;
	private int _lossSuppress = 1;
	private string _topicName = "/29west/tmon";
    private LBMContext _tmonUmContext = null;
    private LBMSource _tmonUmSource = null;
    private string _messageHeader = null;
    private TmonGetTimeOfDay _timeOfDay = null;
    StringBuilder _messageBuffer = new StringBuilder(1025);

    // Getters
    internal int GetLossSuppress() { return _lossSuppress; }
    internal string GetMessageHeader() { return _messageHeader; }
    internal TmonGetTimeOfDay GetTimeOfDay() { return _timeOfDay; }
    internal LBMSource GetTmonUmSource() { return _tmonUmSource; }


    // Constructor
    public TmonContext(LBMContext appUmContext) {
      _objectId = GetNextObjectId();
      _appUmContext = appUmContext;
    }  // TmonContext


    public void Close() {
      _timeOfDay.NewTimeStamp();

      _messageBuffer.Clear();
      _messageBuffer.Append('t'); _messageBuffer.Append(',');
      _messageBuffer.Append(GetMessageHeader());
      _messageBuffer.Append(','); _messageBuffer.Append(_timeOfDay.GetSec());
      _messageBuffer.Append(','); _messageBuffer.Append(_timeOfDay.GetUsec());

      _tmonUmSource.send(Encoding.ASCII.GetBytes(_messageBuffer.ToString()), _messageBuffer.Length, 0);

      Dispose();
    }  // close


    public void InitSender() {
      // Encode app ID.
      _appId = _appUmContext.getAttributeValue("monitor_appid")
          .Replace("\\", "\\1")
          .Replace(",", "\\2");
      _transportOptsStr = _appUmContext.getAttributeValue("monitor_transport_opts");
      _transportOpts = _transportOptsStr.Split(';');
      // Encode context name.
      _appContextName = _appUmContext.getAttributeValue("context_name")
          .Replace("\\", "\\1")
          .Replace(",", "\\2");

      GetMonSettings();  // Get monitoring setting from application context.

      UmCtxCreate();

      UmSrcCreate();
      Thread.Sleep(200);  // Allow TR to work.

      _messageHeader = _appId + ","
          + ThisHostIpAddress() + ","
          + Process.GetCurrentProcess().Id + ","
          + _objectId;

      _timeOfDay = new TmonGetTimeOfDay();

      // Send monitoring message that context is created.
      _messageBuffer.Clear();
      _messageBuffer.Append('T'); _messageBuffer.Append(',');
      _messageBuffer.Append(GetMessageHeader());
      _messageBuffer.Append(','); _messageBuffer.Append(_timeOfDay.GetSec());
      _messageBuffer.Append(','); _messageBuffer.Append(_timeOfDay.GetUsec());
      _messageBuffer.Append(','); _messageBuffer.Append(_appContextName);

      _tmonUmSource.send(Encoding.ASCII.GetBytes(_messageBuffer.ToString()), _messageBuffer.Length, 0);
    }  // InitSender


    public TmonReceiver ReceiverCreate(ReceiverType rcvType, string topicName) {
      TmonReceiver newReceiver = new TmonReceiver(this, rcvType, topicName);
      newReceiver.Init();
      return newReceiver;
    }  // ReceiverCreate


    public TmonSource SourceCreate(string topicName) {
      TmonSource newSource = new TmonSource(this, topicName);
      newSource.Init();
      return newSource;
    }  // SourceCreate


    internal static int GetNextObjectId() {
      return Interlocked.Increment(ref _currentObjectCount);
    }


    /////////////////////////////////////////////////////////////
    // End of APIs.
    /////////////////////////////////////////////////////////////


    // Set up the IDispose interfaces.
    // This is because UM objects need to be
    // closed in the right order.

    public void Dispose()
    {
      Dispose(true);
      GC.SuppressFinalize(this);
    }  // Dispose

    private void Dispose(bool disposing) {
      if (_tmonUmSource != null) {
        _tmonUmSource.flush();
        _tmonUmSource.close();
        _tmonUmSource = null;
      }
      if (_tmonUmContext != null) {
        _tmonUmContext.close();
        _tmonUmContext = null;
      }
    }  // Dispose

    ~TmonContext() {
      try {
        Dispose(false);
      }
      catch(LBMException e) {
        Console.Error.WriteLine(e.StackTrace);
      }
    }  // ~TmonContext


    private void GetMonSettings() {
      // Step through transport options to find topic and config.
      foreach (var opt in _transportOpts) {
        string[] keyVal = opt.Split('=');
        if (keyVal.Length != 2) {
          throw new TmonException("invalid transport option '"+opt+"' in '"+_transportOptsStr+"'");
        }
        if (keyVal[0].Equals("tmon_topic", StringComparison.OrdinalIgnoreCase)) {
          _topicName = keyVal[1];
        }
        else if (keyVal[0].Equals("tmon_loss_suppress", StringComparison.OrdinalIgnoreCase)) {
          _lossSuppress = int.Parse(keyVal[1]);
        }
        else if (keyVal[0].Equals("config", StringComparison.OrdinalIgnoreCase)) {
          _cfgFile = keyVal[1];
        }
      }  // foreach
    }  // GetMonSettings


    private void UmCtxCreate() {
      LBMContextAttributes ctxAttr = new LBMContextAttributes("29west_tmon_context");

      // Configure the tmon context.
      if (_cfgFile != null) {
        ContextAttrSetoptFromFile(ctxAttr, _cfgFile);
      }

      // Any options supplied in transport_opts override
      // Step through transport options to find context opts
      foreach (var opt in _transportOpts) {
        string[] keyVal = opt.Split('=');
        if (keyVal.Length != 2) {
          throw new TmonException("invalid transport option '"+opt+"' in '"+_transportOptsStr+"'");
        }
        string[] scopeOpt = keyVal[0].Split('|');
        if (scopeOpt.Length == 2
            && scopeOpt[0].Equals("context", StringComparison.OrdinalIgnoreCase)) {
          ctxAttr.setValue(scopeOpt[1], keyVal[1]);
        }
      }  // foreach
      ctxAttr.setValue("monitor_appid", "");
      ctxAttr.setValue("monitor_interval", "0");

      _tmonUmContext = new LBMContext(ctxAttr);
    }  // UmCtxCreate


    private void UmSrcCreate() {
      LBMSourceAttributes srcAttr = new LBMSourceAttributes("29west_tmon_context", _topicName);

      // Configure the tmon source.
      if (_cfgFile != null) {
        SourceAttrSetoptFromFile(srcAttr, _cfgFile);
      }

      // Any options supplied in transport_opts override
      // Step through transport options to find source opts
      foreach (var opt in _transportOpts) {
        string[] keyVal = opt.Split('=');
        if (keyVal.Length != 2) {
          throw new TmonException("invalid transport option '"+opt+"' in '"+_transportOptsStr+"'");
        }
        string[] scopeOpt = keyVal[0].Split('|');
        if (scopeOpt.Length == 2
            && scopeOpt[0].Equals("source", StringComparison.OrdinalIgnoreCase)) {
          srcAttr.setValue(scopeOpt[1], keyVal[1]);
        }
      }  // foreach

      LBMTopic topic =  _tmonUmContext.allocTopic(_topicName, srcAttr);
      _tmonUmSource = _tmonUmContext.createSource(topic, null, null, null);
    }  // UmSrcCreate


    // Get this host's IP address (i.e. the interface used for the default route).
    // Thanks to "Mr.Wang from Next Door" for this:
    // https://stackoverflow.com/questions/6803073/get-local-ip-address/27376368#27376368
    private string ThisHostIpAddress() {
      using (Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, 0)) {
        // Note that since this is a UDP socket, "Connect" does not actually send
        // or receive any packets to 8.8.8.8. It just binds to the right interface.
        socket.Connect("8.8.8.8", 65530);
        IPEndPoint endPoint = socket.LocalEndPoint as IPEndPoint;
        return(endPoint.Address.ToString());
      }
    }  // LocalIpAddress


    // Replicate "aux" library's lbmaux_context_attr_setopt_from_file().
    public static unsafe void ContextAttrSetoptFromFile(LBMContextAttributes ctxAttr,
        string fileName) {
      string iLine;
      StringBuilder scope = new StringBuilder(1025);
      StringBuilder option = new StringBuilder(1025);
      StringBuilder value = new StringBuilder(1025);
#if (ANYCPU == false)
      size_t matches;
      size_t confLength;
#else
      IntPtr matches;
      IntPtr confLength;
#endif
      int lineNum = 0;

      System.IO.StreamReader configFile =
          new System.IO.StreamReader(fileName);
      while ((iLine = configFile.ReadLine()) != null) {
        lineNum++;
        // Skip comment and blank lines.
        if (iLine.Length == 0 || Char.IsWhiteSpace(iLine[0])) {
          continue;
        }
        if (iLine.Length > 1024) {
          configFile.Close();
          throw new TmonException("File '"+fileName+"', Line "+lineNum+": too long ("+iLine.Length+")");
        }

        lbm_parse_config_line(iLine, scope, option, value, &matches, &confLength);
        if ((int)matches == 0) {
          continue;  // Blank line, possibly with spaces.
        }
        if ((int)matches != 3 || confLength != iLine.Length) {
          configFile.Close();
          throw new TmonException("File '"+fileName+"', Line "+lineNum+": parse error (matches="+matches+", confLength="+(int)confLength+", line len="+iLine.Length+")");
        }

        // If value is a number with commas, strip the commas.
        string scopeStr = scope.ToString();
        string optionStr = option.ToString();
        string valueStr = value.ToString();
        if (IsDigitsComma(valueStr)) {
          valueStr = valueStr.Replace(",", "");
        }

        // Set context attributes.
        if (scopeStr.Equals("context", StringComparison.OrdinalIgnoreCase)) {
          try {
            ctxAttr.setValue(optionStr, valueStr);
          } catch (Exception e) {
            configFile.Close();
            throw new TmonException("File '"+fileName+"', Line "+lineNum+": setValue error '"+e.Message+"'");
          }
        }
      }  // while

      configFile.Close();
    }  // ContextAttrSetoptFromFile


    // Replicate "aux" library's lbmaux_source_attr_setopt_from_file().
    public static unsafe void SourceAttrSetoptFromFile(LBMSourceAttributes srcAttr,
        string fileName) {
      string iLine;
      StringBuilder scope = new StringBuilder(1025);
      StringBuilder option = new StringBuilder(1025);
      StringBuilder value = new StringBuilder(1025);
#if (ANYCPU == false)
      size_t matches;
      size_t confLength;
#else
      IntPtr matches;
      IntPtr confLength;
#endif
      int lineNum = 0;

      System.IO.StreamReader configFile =
          new System.IO.StreamReader(fileName);
      while ((iLine = configFile.ReadLine()) != null) {
        lineNum++;
        // Skip comment and blank lines.
        if (iLine.Length == 0 || Char.IsWhiteSpace(iLine[0])) {
          continue;
        }
        if (iLine.Length > 1024) {
          configFile.Close();
          throw new TmonException("File '"+fileName+"', Line "+lineNum+": too long ("+iLine.Length+")");
        }

        lbm_parse_config_line(iLine, scope, option, value, &matches, &confLength);
        if ((int)matches == 0) {
          continue;  // Blank line, possibly with spaces.
        }
        if ((int)matches != 3 || confLength != iLine.Length) {
          configFile.Close();
          throw new TmonException("File '"+fileName+"', Line "+lineNum+": parse error (matches="+matches+", confLength="+(int)confLength+", line len="+iLine.Length+")");
        }

        // If value is a number with commas, strip the commas.
        string scopeStr = scope.ToString();
        string optionStr = option.ToString();
        string valueStr = value.ToString();
        if (IsDigitsComma(valueStr)) {
          valueStr = valueStr.Replace(",", "");
        }

        // Set source attributes.
        if (scopeStr.Equals("source", StringComparison.OrdinalIgnoreCase)) {
          try {
            srcAttr.setValue(optionStr, valueStr);
          } catch (Exception e) {
            configFile.Close();
            throw new TmonException("File '"+fileName+"', Line "+lineNum+": setValue error '"+e.Message+"'");
          }
        }
      }  // while

      configFile.Close();
    }  // SourceAttrSetoptFromFile


    static bool IsDigitsComma(string s) {
      foreach (char c in s) {
        if (c == ',') {
          continue;
        }
        else if (c < '0' || c > '9') {
          return false;
        }
      }

      return true;
    }  // IsDigitsComma
  }  // TmonContext
}  // com.latencybusters.Tmon
