#!/bin/sh

# Station
STATION="/0"
# First connector
FIRST_CONNECTOR="/1"
# Second connector
SECOND_CONNECTOR="/2"
# Third connector
THIRD_CONNECTOR="/3"
# Fourth connector
FOURTH_CONNECTOR="/4"
# App logs
APP_LOG="/app.log"

# Logs path
LOGS_DIR="/ocppclient/log"
# Station path
STATION_DIR="$LOGS_DIR$STATION"
# First connector path
FIRST_CONNECTOR_DIR="$LOGS_DIR$FIRST_CONNECTOR"
# Second connector path
SECOND_CONNECTOR_DIR="$LOGS_DIR$SECOND_CONNECTOR"
# Third connector path
THIRD_CONNECTOR_DIR="$LOGS_DIR$THIRD_CONNECTOR"
# Fourth connector path
FOURTH_CONNECTOR_DIR="$LOGS_DIR$FOURTH_CONNECTOR"
# App log path
APP_LOG_PATH="$LOGS_DIR$APP_LOG"

# Days count greather then remove log files
# HALF YEAR
DAYS=185

if [ $# -gt 0 ]; then
  DAYS=$1
else
  DAYS=185
fi

# Find and remove empty dirs
function removeEmptyDirs() {
  local parent_dir="$1"

  for year in $parent_dir/*; do
    for month in $year/*; do
      for day in $month/*; do
        removeEmptyDir $day
      done
      removeEmptyDir $month
    done
    removeEmptyDir $year
  done
}

# Remove empty dirs
function removeEmptyDir() {
  local dir=$1
  if [ -d "$dir" ]; then
    if [ ! "$(ls -A $dir)" ]; then
      rm -rf $dir
    fi
  fi
}

# Remove old log files
function removeOldFiles() {
  # Dir for action
  DIR=$1
  # Days count older then remove files
  DAYS=$2

  DAY_IN_SEC=86400

  cur_time=$(date +%s)

  find $DIR -type f -print | while read file; do
    mod_time=$(stat "$file" | grep "Modify" | awk '{print $2}')
    mod_time_sec=$(date -d "$mod_time" +%s)
    days_diff=$(((cur_time - mod_time_sec) / $DAY_IN_SEC))

    if [ $days_diff -gt $DAYS ]; then
      echo "$file"
      rm -rf "$file"
    fi
  done
}

# Rotate log app
function rotateAppLog() {
  if [ -z $(cat $APP_LOG_PATH) ]; then
    echo "EMPTY"
  else
    DATE=$(date "+%d-%m-%Y_%H-%M-%S")

    echo "NOT EMPTY"
    # cp app old log to backup
    cp $APP_LOG_PATH ${APP_LOG_PATH}_${DATE}
    # clear log
    echo "" > $APP_LOG_PATH
  fi
}

# Rotate log app
rotateAppLog

# Remove old files
removeOldFiles $STATION_DIR $DAYS
removeOldFiles $FIRST_CONNECTOR_DIR $DAYS
removeOldFiles $SECOND_CONNECTOR_DIR $DAYS
removeOldFiles $THIRD_CONNECTOR_DIR $DAYS
removeOldFiles $FOURTH_CONNECTOR_DIR $DAYS

# Remove files in LOGS_DIR older then 6 day
removeOldFiles $LOGS_DIR/ $DAYS

# Remove empty dirs
removeEmptyDirs $FIRST_CONNECTOR_DIR
removeEmptyDirs $SECOND_CONNECTOR_DIR
removeEmptyDirs $THIRD_CONNECTOR_DIR
removeEmptyDirs $FOURTH_CONNECTOR_DIR