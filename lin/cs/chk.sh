#!/bin/sh
# chk.sh

perl -e '
  use strict;
  use warnings;
  my $oline = "";
  # Main loop; read each line in each file.
  while (<>) {
    chomp;  # remove trailing \n
    s/\r//g;
    if (/^\s/) {
      s/^  time=.*$/  time=a/;
      s/^  ip=.*$/  ip=b/;
      s/^  pid=.*$/  pid=c/;
      s/^  src_str=.*$/  src_str=d/;
      $oline .= $_;
    } else {
      if ($oline =~ /^Tmon:/) {
        print "$oline\n";
      }
      $oline = $_;
    }
  }
  if (length($oline) > 0) {
    if ($oline =~ /^Tmon:/) {
      print "$oline\n";
    }
  }' <lbmtmon.log >lbmtmon.tst

if diff lbmtmon.chk lbmtmon.tst;
then echo "ok"
else :
  echo "Error"
  exit 1
fi
