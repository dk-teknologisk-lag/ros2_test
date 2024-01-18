#!/bin/bash

. install/setup.bash && ros2 run cpp_pubsub talker --ros-args -p freq:=10 -p bytesize:=1000000