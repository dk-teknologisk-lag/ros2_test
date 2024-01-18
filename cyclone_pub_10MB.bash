#!/bin/bash

. install/setup.bash && RMW_IMPLEMENTATION=rmw_cyclonedds_cpp ros2 run cpp_pubsub talker --ros-args -p freq:=10 -p bytesize:=10000000