#!/bin/sh

# Script for uploading using sftp
if [ $# -ne 0 ]; then
    bash upload.sh $1 robot
    bash connect.sh $1
fi;
