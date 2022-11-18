#!/usr/bin/env bash

sudo echo 'D' > /proc/lincoln/cmd
echo "Hi, the keyboard is disable for 3 seconds"
sleep 3
sudo echo 'E' > /proc/lincoln/cmd
sudo echo 'R' > /proc/lincoln/cmd
sudo echo 'E' > /proc/lincoln/cmd
