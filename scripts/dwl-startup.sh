#!/bin/bash

# Kill already running duplicate process
_ps="notif_launcher mpd"
for _prs in $_ps; do
    if [ "$(pidof "${_prs}")" ]; then
         killall -9 "${_prs}"
    fi  
 done

# Start our applications
swaybg --output '*' --mode center --image ~/Pictures/lainfence.png &
foot --server &
/usr/bin/rm -rf ~/Pictures/mpvcache/* &
/usr/bin/rm -rf ~/.cache/mpv/* &
(
  dbus-launch --exit-with-session start-pipewire restart
) &
dwl-bar &
(
  while ! pactl list short sinks | grep -q .; do
    sleep 0.1
    done
    someblocks -s $XDG_RUNTIME_DIR/dwl-bar-0
) &
tiramisu_notifs &
notif_launcher &
mpd &

exec dbus-update-environment WAYLAND_DISPLAY 
