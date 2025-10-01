#!/bin/bash

# Проверяем, что передан один аргумент - порт
if [ "$#" -ne 1 ]; then
  echo "Использование: $0 <PORT>"
  exit 1
fi

port=$1

# Проверяем занят ли порт с помощью lsof
if lsof -i :"$port" > /dev/null 2>&1; then
  echo "Порт $port занят"
else
  echo "Порт $port свободен"
fi
