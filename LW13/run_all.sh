#!/bin/bash

#Пакетный запуск сервера и клиентов лабораторной работы 13

if [ $# -lt 5 ]
then
echo "Usage:  <Network interface name (for example: eth0)> <Host name> <TCP port number> <UDP port number 1> <UDP port number 2>";
exit -1;
fi

INTERFACENAME=$1;
HOSTNAME=$2;
TCPPORT=$3;
UDPPORT1=$4;
UDPPORT2=$5;

xterm -geometry 100x30+0+0 -e "./server  $INTERFACENAME $HOSTNAME $TCPPORT $UDPPORT1 $UDPPORT2; bash" &
xterm -geometry 100x30-0+0 -e "./client1 $HOSTNAME $TCPPORT $UDPPORT1; bash" &
xterm -geometry 100x30-0-0 -e "./client2 $HOSTNAME $TCPPORT $UDPPORT2; bash"