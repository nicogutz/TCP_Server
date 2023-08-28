#!/usr/bin/ bash
port=5610
# start 3 clients

./sensor_node 15 0.1 127.0.0.1 $port &
./sensor_node 2 0.1 127.0.0.1 $port &
./sensor_node 37 0.1 127.0.0.1 $port &

sleep 2
killall sensor_node
