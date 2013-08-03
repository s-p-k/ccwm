ccwm
====

C CLI Wlan Manager

ccwm allows you to easily connect to any wlan access point.
There is no need for configuration.
This is work in progress but its already functional.

**Command line options:**
Usage: ccwm [options] -o/-a/-w/-W

-h              Show help

-i [interface]  Set interface (default: wlan0)
-e [ssid]       Set SSID
-f [freq]       Set frequency
-k [key]        Set key/passphrase

-s              Scan access points
-S              Scan access points verbosely
-I              Show interface info

-o              Connect to ESS access point
-a              Connect to IBSS access point
-w              Connect to WEP encrypted access point
-W              Connect to WPA encrypted access point

changelog
====
03.08.2013 - First functional release.
