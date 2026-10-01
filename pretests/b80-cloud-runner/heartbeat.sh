#!/bin/sh
# B80 / T6 part B: heartbeat. Appends one line per minute to the log given as $1.
# Uses no CPU between beats (it waits in sleep). Logs boot_id so a container swap is visible.
LOG="$1"
echo "start $(date -u +%Y-%m-%dT%H:%M:%SZ) pid=$$ boot=$(cat /proc/sys/kernel/random/boot_id) uptime_s=$(cut -d' ' -f1 /proc/uptime)" >> "$LOG"
while true; do
  sleep 60
  echo "beat $(date -u +%Y-%m-%dT%H:%M:%SZ) pid=$$ boot=$(cat /proc/sys/kernel/random/boot_id) uptime_s=$(cut -d' ' -f1 /proc/uptime)" >> "$LOG"
done
