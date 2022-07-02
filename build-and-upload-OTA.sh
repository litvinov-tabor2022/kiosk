#!/bin/bash

function start_telnet() {
  n=0
   until [ $n -ge 5 ]
   do
      telnet 192.168.0.1 && break
      n=$[$n+1]
      sleep 1
   done
}

pio run --target upload -e ota && start_telnet
