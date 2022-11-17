#!/usr/bin/env bash

sudo echo 'D' > /proc/lincoln/cmd
echo "Hi, the keyboard is disable for 7 seconds"
sleep 7 
sudo echo 'E' > /proc/lincoln/cmd
sudo echo 'R' > /proc/lincoln/cmd
