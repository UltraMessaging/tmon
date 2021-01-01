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

namespace com.latencybusters.Tmon {
  public class TmonGetTimeOfDay {
    private DateTime _timeStamp;
    private long _sec = 0;
    private long _usec = 0;

    // Constructor
    public TmonGetTimeOfDay() {
      NewTimeStamp();
    }

    // Getters.
    public long GetSec() { return _sec; }
    public long GetUsec() { return _usec; }


    public void NewTimeStamp() {
      _timeStamp = DateTime.UtcNow;
      _sec = ((DateTimeOffset)_timeStamp).ToUnixTimeSeconds();
      _usec = ((_timeStamp.Ticks % TimeSpan.TicksPerSecond) * 1000000) / TimeSpan.TicksPerSecond;
    }  // TakeTimeStamp
  }  // TmonGetTimeOfDay
}  // com.latencybusters.Tmon
