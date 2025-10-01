#!/bin/sh

PATH_LOGS=/ocppclient/log/
ALL_LOGS=/ocppclient/log/all_logs

mkdir -p "$ALL_LOGS" 1> /dev/null 2> /dev/null

find $PATH_LOGS -type f -print | while read file; do
  cp -p "$file" "$ALL_LOGS" 1> /dev/null 2> /dev/null
done
