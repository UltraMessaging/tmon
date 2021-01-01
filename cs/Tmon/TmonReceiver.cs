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
using System.Runtime.InteropServices;
using System.Text;
using com.latencybusters.lbm;

namespace com.latencybusters.Tmon {
  public enum ReceiverType {
    Regular,
    Wildcard
  }

  public class TmonReceiver {
    private int _objectId = 0;
    private TmonContext _parentTmonContext = null;
    private ReceiverType _rcvType;
    private string _topicName;
    private LBMSource _tmonUmSource = null;
    private TmonGetTimeOfDay _timeOfDay = null;
    private StringBuilder _messageBuffer = new StringBuilder(1025);


    // Getters
    internal int GetObjectId() { return _objectId; }
    internal string GetTopicName() { return _topicName; }


    // Constructor
    public TmonReceiver(TmonContext parentTmonContext, ReceiverType rcvType, string topicName) {
      _objectId = TmonContext.GetNextObjectId();
      _parentTmonContext = parentTmonContext;
      _rcvType = rcvType;
      // Encode topic.
      _topicName = topicName.Replace("\\", "\\1").Replace(",", "\\2");
    }  // TmonReceiver


    public void Init() {
      _tmonUmSource = _parentTmonContext.GetTmonUmSource();
      _timeOfDay = new TmonGetTimeOfDay();

      _messageBuffer.Clear();
      if (_rcvType == ReceiverType.Regular) {
        _messageBuffer.Append('R');
      }
      else if (_rcvType == ReceiverType.Wildcard) {
        _messageBuffer.Append('W');
      }
      else {
        throw new TmonException("invalid rcvType "+(int)_rcvType);
      }
      _messageBuffer.Append(',');
      _messageBuffer.Append(_parentTmonContext.GetMessageHeader());
      _messageBuffer.Append(','); _messageBuffer.Append(_timeOfDay.GetSec());
      _messageBuffer.Append(','); _messageBuffer.Append(_timeOfDay.GetUsec());
      _messageBuffer.Append(','); _messageBuffer.Append(_objectId);
      _messageBuffer.Append(','); _messageBuffer.Append(_topicName);

      _tmonUmSource.send(Encoding.ASCII.GetBytes(_messageBuffer.ToString()), _messageBuffer.Length, 0);
    }  // Init


    public TmonConnection ConnectionCreate(string sourceName) {
      TmonConnection newConnection = new TmonConnection(this, _parentTmonContext, sourceName);
      newConnection.Init();
      return newConnection;
    }  // ConnectionCreate


    public void Close() {
      _timeOfDay.NewTimeStamp();

      _messageBuffer.Clear();
      if (_rcvType == ReceiverType.Regular) {
        _messageBuffer.Append('r');
      }
      else if (_rcvType == ReceiverType.Wildcard) {
        _messageBuffer.Append('w');
      }
      else {
        throw new TmonException("invalid rcvType "+(int)_rcvType);
      }
      _messageBuffer.Append(',');
      _messageBuffer.Append(_parentTmonContext.GetMessageHeader());
      _messageBuffer.Append(','); _messageBuffer.Append(_timeOfDay.GetSec());
      _messageBuffer.Append(','); _messageBuffer.Append(_timeOfDay.GetUsec());
      _messageBuffer.Append(','); _messageBuffer.Append(_objectId);

      _tmonUmSource.send(Encoding.ASCII.GetBytes(_messageBuffer.ToString()), _messageBuffer.Length, 0);
    }  // Close
  }  // TmonReceiver
}  // com.latencybusters.Tmon
