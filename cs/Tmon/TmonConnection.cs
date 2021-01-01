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
  public class TmonConnection {
    private int _objectId = 0;
    private TmonReceiver _parentTmonReceiver = null;
    private TmonContext _parentTmonContext = null;
	private string _sourceName = null;
    private LBMSource _tmonUmSource = null;
    private TmonGetTimeOfDay _timeOfDay = null;
    private StringBuilder _reportBuffer = new StringBuilder(1025);
    private long _msgCount = 0;
    private long _lossReportSec = 0;
    private long _lossCount = 0;
    private long _burstCount = 0;
    private long _datagramsLost = 0;
    private long _lastSequenceNum = 0;


    // Constructor
    public TmonConnection(TmonReceiver parentTmonReceiver, TmonContext parentTmonContext,
        string sourceName) {
      _objectId = TmonContext.GetNextObjectId();
      _parentTmonReceiver = parentTmonReceiver;
      _parentTmonContext = parentTmonContext;
      // Encode source string.
      _sourceName = sourceName.Replace("\\", "\\1").Replace(",", "\\2");
    }  // TmonConnection


    public void Init() {
      _tmonUmSource = _parentTmonContext.GetTmonUmSource();
      _timeOfDay = new TmonGetTimeOfDay();

      _reportBuffer.Clear();
      _reportBuffer.Append('C'); _reportBuffer.Append(',');
      _reportBuffer.Append(_parentTmonContext.GetMessageHeader());
      _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetSec());
      _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetUsec());
      _reportBuffer.Append(','); _reportBuffer.Append(_objectId);
      _reportBuffer.Append(','); _reportBuffer.Append(_parentTmonReceiver.GetObjectId());
      _reportBuffer.Append(','); _reportBuffer.Append(_sourceName);

      _tmonUmSource.send(Encoding.ASCII.GetBytes(_reportBuffer.ToString()), _reportBuffer.Length, 0);
    }  // Init


    private void ReceiverBos(LBMMessage msg) {
      _timeOfDay.NewTimeStamp();

      _reportBuffer.Clear();
      _reportBuffer.Append('B'); _reportBuffer.Append(',');
      _reportBuffer.Append(_parentTmonContext.GetMessageHeader());
      _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetSec());
      _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetUsec());
      _reportBuffer.Append(','); _reportBuffer.Append(_objectId);
      _reportBuffer.Append(','); _reportBuffer.Append(msg.topicName().Replace("\\", "\\1").Replace(",", "\\2"));

      _tmonUmSource.send(Encoding.ASCII.GetBytes(_reportBuffer.ToString()), _reportBuffer.Length, 0);
    }  // ReceiverBos

    private void ReceiverEos(LBMMessage msg) {
      _timeOfDay.NewTimeStamp();

      _reportBuffer.Clear();
      _reportBuffer.Append('E'); _reportBuffer.Append(',');
      _reportBuffer.Append(_parentTmonContext.GetMessageHeader());
      _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetSec());
      _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetUsec());
      _reportBuffer.Append(','); _reportBuffer.Append(_objectId);
      _reportBuffer.Append(','); _reportBuffer.Append(_msgCount);
      _reportBuffer.Append(','); _reportBuffer.Append(_lossCount);
      _reportBuffer.Append(','); _reportBuffer.Append(_burstCount);
      _reportBuffer.Append(','); _reportBuffer.Append(_datagramsLost);
      _reportBuffer.Append(','); _reportBuffer.Append(msg.topicName().Replace("\\", "\\1").Replace(",", "\\2"));

      _tmonUmSource.send(Encoding.ASCII.GetBytes(_reportBuffer.ToString()), _reportBuffer.Length, 0);
    }  // ReceiverEos

    private void ReceiverLoss(LBMMessage msg) {
      _timeOfDay.NewTimeStamp();

      _lossCount++;
      long burstSize = msg.sequenceNumber() - _lastSequenceNum;
      _lastSequenceNum = msg.sequenceNumber();

      // Send no more than one loss/burst report within any 10-second period.
      if ((_timeOfDay.GetSec() - _lossReportSec) >= _parentTmonContext.GetLossSuppress()) {
        _reportBuffer.Clear();
        _reportBuffer.Append('L'); _reportBuffer.Append(',');
        _reportBuffer.Append(_parentTmonContext.GetMessageHeader());
        _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetSec());
        _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetUsec());
        _reportBuffer.Append(','); _reportBuffer.Append(_objectId);
        _reportBuffer.Append(','); _reportBuffer.Append(msg.sequenceNumber());

        _tmonUmSource.send(Encoding.ASCII.GetBytes(_reportBuffer.ToString()), _reportBuffer.Length, 0);
        _lossReportSec = _timeOfDay.GetSec();  /* Remember when the report was made. */
      }
    }  // ReceiverLoss

    private void ReceiverBurstLoss(LBMMessage msg) {
      _timeOfDay.NewTimeStamp();

      _burstCount++;
      long burstSize = msg.sequenceNumber() - _lastSequenceNum;
      _lastSequenceNum = msg.sequenceNumber();

      // Send no more than one loss/burst report within any 10-second period.
      if ((_timeOfDay.GetSec() - _lossReportSec) >= _parentTmonContext.GetLossSuppress()) {
        _reportBuffer.Clear();
        _reportBuffer.Append('l'); _reportBuffer.Append(',');
        _reportBuffer.Append(_parentTmonContext.GetMessageHeader());
        _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetSec());
        _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetUsec());
        _reportBuffer.Append(','); _reportBuffer.Append(_objectId);
        _reportBuffer.Append(','); _reportBuffer.Append(msg.sequenceNumber());
        _reportBuffer.Append(','); _reportBuffer.Append(burstSize);

        _tmonUmSource.send(Encoding.ASCII.GetBytes(_reportBuffer.ToString()), _reportBuffer.Length, 0);
        _lossReportSec = _timeOfDay.GetSec();  /* Remember when the report was made. */
      }
    }  // receiverBurstLoss

    public void ReceiverEvent(LBMMessage msg) {
      switch (msg.type()) {
        case LBM.MSG_DATA:
          _msgCount++;
          _lastSequenceNum = msg.sequenceNumber();
          break;

        case LBM.MSG_BOS:
          ReceiverBos(msg);
          break;

        case LBM.MSG_EOS:
          ReceiverEos(msg);
          break;

        case LBM.MSG_UNRECOVERABLE_LOSS:
          ReceiverLoss(msg);
          break;

        case LBM.MSG_UNRECOVERABLE_LOSS_BURST:
          ReceiverBurstLoss(msg);
          break;
      }
    }


    public void Close() {
      _timeOfDay.NewTimeStamp();

      _reportBuffer.Clear();
      _reportBuffer.Append('c'); _reportBuffer.Append(',');
      _reportBuffer.Append(_parentTmonContext.GetMessageHeader());
      _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetSec());
      _reportBuffer.Append(','); _reportBuffer.Append(_timeOfDay.GetUsec());
      _reportBuffer.Append(','); _reportBuffer.Append(_objectId);
      _reportBuffer.Append(','); _reportBuffer.Append(_msgCount);
      _reportBuffer.Append(','); _reportBuffer.Append(_lossCount);
      _reportBuffer.Append(','); _reportBuffer.Append(_burstCount);
      _reportBuffer.Append(','); _reportBuffer.Append(_datagramsLost);

      _tmonUmSource.send(Encoding.ASCII.GetBytes(_reportBuffer.ToString()), _reportBuffer.Length, 0);
    }  // Close
  }  // TmonConnection
}  // com.latencybusters.Tmon
