//in addition to this i also commented out focusmon, tagmon, togglebar, togglegaps and dirtomon in dwl.c
//i applied btrtile, focusdir, rotatetag, warpcursor, pertag, gaps, ipc and ipcpertag patches
/* Taken from https://github.com/djpohly/dwl/issues/466 */
#define COLOR(hex)    { ((hex >> 24) & 0xFF) / 255.0f, \
                        ((hex >> 16) & 0xFF) / 255.0f, \
                        ((hex >> 8) & 0xFF) / 255.0f, \
                        (hex & 0xFF) / 255.0f }
/* appearance */
static const int sloppyfocus               = 1;  /* focus follows mouse */
static const int bypass_surface_visibility = 0;  /* 1 means idle inhibitors will disable idle tracking even if it's surface isn't visible  */
static const int smartgaps                 = 0;  /* 1 means no outer gap when there is only one window */
static int gaps                            = 1;  /* 1 means gaps between windows are added */
static const unsigned int gappx            = 10; /* gap pixel between windows */
static const unsigned int borderpx         = 1;  /* border pixel of windows */
static const float rootcolor[]             = COLOR(0x00000000);
static const float bordercolor[]           = COLOR(0x00000000);
static const float focuscolor[]            = COLOR(0x00000000);
static const float urgentcolor[]           = COLOR(0x00000000);
/* This conforms to the xdg-protocol. Set the alpha to zero to restore the old behavior */
static const float fullscreen_bg[]         = {0.1f, 0.1f, 0.1f, 1.0f}; /* You can also use glsl colors */
static const float resize_factor           = 0.0002f; /* Resize multiplier for mouse resizing, depends on mouse sensivity. */
static const uint32_t resize_interval_ms   = 16; /* Resize interval depends on framerate and screen refresh rate. */

enum Direction { DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN };
/* tagging - TAGCOUNT must be no greater than 31 */
#define TAGCOUNT (10)

/* logging */
static int log_level = WLR_ERROR;

/* NOTE: ALWAYS keep a rule declared even if you don't use rules (e.g leave at least one example) */
static const Rule rules[] = {
        /* app_id             title       tags mask     isfloating   monitor */
        /* examples: */
        { "Gimp_EXAMPLE",     NULL,       0,            1,           -1 }, /* Start on currently visible tags floating, not tiled */
        { "firefox_EXAMPLE",  NULL,       1 << 8,       0,           -1 }, /* Start on ONLY tag "9" */
};

/* layout(s) */
static const Layout layouts[] = {
        /* symbol     arrange function */
        { "|w|",      btrtile },
        { "[]=",      tile },
        { "><>",      NULL },    /* no layout function means floating behavior */
        { "[M]",      monocle },
};

/* monitors */
/* (x=-1, y=-1) is reserved as an "autoconfigure" monitor position indicator
 * WARNING: negative values other than (-1, -1) cause problems with Xwayland clients
 * https://gitlab.freedesktop.org/xorg/xserver/-/issues/899
*/
/* NOTE: ALWAYS add a fallback rule, even if you are completely sure it won't be used */
static const MonitorRule monrules[] = {
        /* name       mfact  nmaster scale layout       rotate/reflect                x    y */
        /* example of a HiDPI laptop monitor:
        { "eDP-1",    0.5f,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
        */
        /* defaults */
        { NULL,       0.55f, 1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
};

/* keyboard */
static const struct xkb_rule_names xkb_rules = {
        // can specify fields: rules, model, layout, variant, options */
        /* example:
        .options = "ctrl:nocaps",
        */
.options = NULL,
};

static const int repeat_rate = 25;
static const int repeat_delay = 600;

/* Trackpad */
static const int tap_to_click = 1;
static const int tap_and_drag = 1;
static const int drag_lock = 1;
static const int natural_scrolling = 1;
static const int disable_while_typing = 1;
static const int left_handed = 0;
static const int middle_button_emulation = 0;
/* You can choose between:
rIBINPUT_CONFIG_SCROLL_NO_SCROLL
LIBINPUT_CONFIG_SCROLL_2FG
LIBINPUT_CONFIG_SCROLL_EDGE
LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN
*/
static const enum libinput_config_scroll_method scroll_method = LIBINPUT_CONFIG_SCROLL_2FG;

/* You can choose between:
LIBINPUT_CONFIG_CLICK_METHOD_NONE
LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS
LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
*/
static const enum libinput_config_click_method click_method = LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER;

/* You can choose between:
LIBINPUT_CONFIG_SEND_EVENTS_ENABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE
*/
static const uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;

/* You can choose between:
LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT
LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
*/
static const enum libinput_config_accel_profile accel_profile = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
static const double accel_speed = 0.0;

/* You can choose between:
LIBINPUT_CONFIG_TAP_MAP_LRM -- 1/2/3 finger tap maps to left/right/middle
LIBINPUT_CONFIG_TAP_MAP_LMR -- 1/2/3 finger tap maps to left/middle/right
*/
static const enum libinput_config_tap_button_map button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;

/* If you want to use the windows key for MODKEY, use WLR_MODIFIER_LOGO */
#define MODKEY WLR_MODIFIER_LOGO

#define TAGKEYS(KEY,SKEY,TAG) \
        { MODKEY,                    KEY,            view,            {.ui = 1 << TAG} }, \
        { MODKEY|WLR_MODIFIER_CTRL,  KEY,            toggleview,      {.ui = 1 << TAG} }, \
        { MODKEY|WLR_MODIFIER_SHIFT, SKEY,           tag,             {.ui = 1 << TAG} }, \
        { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT,SKEY,toggletag, {.ui = 1 << TAG} }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static const char *termcmd[] = { "foot", NULL };
static const char *dmenucmd[] = { "tofi-drun", "--drun-launch=true", NULL };
static const char *volup[]   = { "sh", "-c", "pactl set-sink-volume @DEFAULT_SINK@ +1% && pkill -RTMIN+1 someblocks", NULL };
static const char *voldown[] = { "sh", "-c", "pactl set-sink-volume @DEFAULT_SINK@ -1% && pkill -RTMIN+1 someblocks", NULL };
static const char *volmute[] = { "sh", "-c", "pactl set-sink-mute @DEFAULT_SINK@ toggle && pkill -RTMIN+1 someblocks", NULL };
static const char *micmute[] = { "pactl", "set-source-mute", "@DEFAULT_SOURCE@", "toggle", NULL };
static const char *brup[]   = { "sh", "-c", "sudo brightnessctl set 1%+ && pkill -SIGRTMIN+2 someblocks", NULL };
static const char *brdown[] = { "sh", "-c", "sudo brightnessctl set 1%- && pkill -SIGRTMIN+2 someblocks", NULL };
static const char *lockcmd[] = { "wiredlock", NULL };
static const char *shutdowncmd[] = { "sudo", "openrc-shutdown", "-p", "now", NULL };
static const char *rebootcmd[]   = { "sudo", "reboot", NULL };
static const char *screenshot[]        = { "wayscreenshot", NULL };
static const char *screenshot_select[] = { "wayscreenshot", "select", NULL };
static const char *screenrecord[]      = { "wayscreenrecord", NULL };
static const char *screenrecord_select[] = { "wayscreenrecord", "select", NULL };
static const char *screenrecord_kill[] = { "wayscreenrecord", "kill", NULL };

static const Key keys[] = {
        /* Note that Shift changes certain key codes: c -> C, 2 -> at, etc. */
        /* modifier                  key                 function        argument */
        { MODKEY,                    XKB_KEY_d,          spawn,          {.v = dmenucmd} }, /*menu*/
        { MODKEY,                    XKB_KEY_Return,     spawn,          {.v = termcmd} }, /*foot*/
        { MODKEY,                    XKB_KEY_Right,      focusstack,     {.i = +1} }, //focus right
        { MODKEY,                    XKB_KEY_Left,       focusstack,     {.i = -1} }, //focus left
        { MODKEY,                    XKB_KEY_i,          incnmaster,     {.i = +1} }, //dwm tile add to master
        { MODKEY,                    XKB_KEY_d,          incnmaster,     {.i = -1} }, //dwm tile remove from master
        { MODKEY,                    XKB_KEY_h,          setmfact,       {.f = -0.05f} }, //dwm tile decrease master size
        { MODKEY,                    XKB_KEY_l,          setmfact,       {.f = +0.05f} }, //dwm tile increase master size 
        { MODKEY,                    XKB_KEY_z,          zoom,           {0} }, //dwm tile switches master 
        { MODKEY,                    XKB_KEY_q,          killclient,     {0} }, //kills window 
        { MODKEY,                    XKB_KEY_t,          setlayout,      {.v = &layouts[0]} }, //btrtile
        { MODKEY,                    XKB_KEY_y,          setlayout,      {.v = &layouts[1]} }, //dwm tile
        { MODKEY,                    XKB_KEY_u,          setlayout,      {.v = &layouts[2]} }, //float 
        { MODKEY,                    XKB_KEY_space,      setlayout,      {0} }, //switches between dwm tile and btrtile
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_space,      togglefloating, {0} }, //makes window float
        { MODKEY,                    XKB_KEY_f,          togglefullscreen, {0} }, //makes window fullscreen
        /*stuff for monitors*/
        /*{ MODKEY,                    XKB_KEY_comma,      focusmon,       {.i = WLR_DIRECTION_LEFT} },
        { MODKEY,                    XKB_KEY_period,     focusmon,       {.i = WLR_DIRECTION_RIGHT} },
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_less,       tagmon,         {.i = WLR_DIRECTION_LEFT} },
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_greater,    tagmon,         {.i = WLR_DIRECTION_RIGHT} },*/
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Up,         swapclients,    {.i = DIR_UP} },
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Down,       swapclients,    {.i = DIR_DOWN} },
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Right,      swapclients,    {.i = DIR_RIGHT} },
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Left,       swapclients,    {.i = DIR_LEFT} },
        { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Right,      setratio_h,     {.f = +0.025f} },
        { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Left,       setratio_h,     {.f = -0.025f} },
        { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Up,         setratio_v,     {.f = -0.025f} },
        { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Down,       setratio_v,     {.f = +0.025f} },
        TAGKEYS(          XKB_KEY_1, XKB_KEY_exclam,                     0),
        TAGKEYS(          XKB_KEY_2, XKB_KEY_at,                         1),
        TAGKEYS(          XKB_KEY_3, XKB_KEY_numbersign,                 2),
        TAGKEYS(          XKB_KEY_4, XKB_KEY_dollar,                     3),
        TAGKEYS(          XKB_KEY_5, XKB_KEY_percent,                    4),
        TAGKEYS(          XKB_KEY_6, XKB_KEY_asciicircum,                5),
        TAGKEYS(          XKB_KEY_7, XKB_KEY_ampersand,                  6),
        TAGKEYS(          XKB_KEY_8, XKB_KEY_asterisk,                   7),
        TAGKEYS(          XKB_KEY_9, XKB_KEY_parenleft,                  8),
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_E,          quit,           {0} },
        TAGKEYS(          XKB_KEY_0, XKB_KEY_parenright,                 9),
        { 0, XKB_KEY_F3, spawn, {.v = volup} },
        { 0, XKB_KEY_F2, spawn, {.v = voldown} },
        { 0, XKB_KEY_F1, spawn, {.v = volmute} },
        { 0, XKB_KEY_F4, spawn, {.v = micmute} },
        { MODKEY, XKB_KEY_F5, spawn, {.v = brdown} },
        { 0, XKB_KEY_F6, spawn, {.v = brup} },
        { MODKEY, XKB_KEY_x, spawn, {.v = lockcmd} },
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_X, spawn, {.v = shutdowncmd} },
        { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Z, spawn, {.v = rebootcmd} },
        { 0,                      XKB_KEY_Print, spawn, {.v = screenshot} },
        { WLR_MODIFIER_SHIFT,     XKB_KEY_Print, spawn, {.v = screenshot_select} },
        { MODKEY,                   XKB_KEY_Print, spawn, {.v = screenrecord} },
        { MODKEY|WLR_MODIFIER_SHIFT,                    XKB_KEY_Print, spawn, {.v = screenrecord_select} },
        { WLR_MODIFIER_CTRL,  XKB_KEY_Print, spawn, {.v = screenrecord_kill} },
        /* Ctrl-Alt-Backspace and Ctrl-Alt-Fx used to be handled by X server */
        /* Ctrl-Alt-Fx is used to switch to another VT, if you don't know what a VT is
         * do not remove them.
         */
#define CHVT(n) { WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_XF86Switch_VT_##n, chvt, {.ui = (n)} }
        CHVT(1), CHVT(2), CHVT(3), CHVT(4), CHVT(5), CHVT(6),
        CHVT(7), CHVT(8), CHVT(9), CHVT(10), CHVT(11), CHVT(12),
};

static const Button buttons[] = {
        { MODKEY, BTN_LEFT,   moveresize,     {.ui = CurMove} },
        { MODKEY|WLR_MODIFIER_CTRL,      BTN_LEFT,   moveresize, {.ui = CurResize} },
};
