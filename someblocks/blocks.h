//i use someblocks with dwl-bar and to remove the title thing in dwl bar, i commented out the inside of the bar_set_title function in bar.c

//Modify this file to change what commands output to your statusbar, and recompile using the make command.
static const Block blocks[] = {
        /*Icon*/        /*Command*/             /*Update Interval*/     /*Update Signal*/

        // Volume percentage, no trailing space
{
    "", 
    "sink=$(pactl list short sinks | awk 'NR==1 {print $2}'); \
     if [ -n \"$sink\" ]; then \
         vol=$(pactl get-sink-volume \"$sink\" | awk '{print $5}' | head -1); \
         if pactl get-sink-mute \"$sink\" | grep -q yes; then \
             echo -n \"[Vol: $vol (muted)]\"; \
         else \
             echo -n \"[Vol: $vol]\"; \
         fi; \
     else \
         echo -n \"[Vol: N/A]\"; \
     fi", 
    0, 
    1   
},
 
   {"", "echo -n \"[Lum: $(brightnessctl get | awk -v max=$(brightnessctl max) '{printf \"%.0f%%\", $1*100/max}')]\"", 0, 2},

   {"", "capacity=$(cat /sys/class/power_supply/BAT0/capacity); status=$(cat /sys/class/power_supply/BAT0/status); if [ \"$status\" = \"Charging\" ]; then echo -n \"[Btt: $capacity% ($status)]\"; else echo -n \"[Btt: $capacity%]\"; fi", 10, 0},

  {"", "echo -n \"[$(date '+%d/%m/%Y (%a) %H:%M:%S')]\"", 1, 0},

        /* Updates whenever "pkill -SIGRTMIN+10 someblocks" is ran */
        /* {"", "date '+%b %d (%a) %I:%M%p'",                                   0,              10}, */
};




//sets delimeter between status commands. NULL character ('\0') means no delimeter.
static char delim[] = " ";
static unsigned int delimLen = 5;
