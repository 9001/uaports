chmod 666 /dev/bus/usb/*/*
modprobe uinput
chmod 666 /dev/uinput 
start xboxdrv until it doesn't crash with "pure virtual method called"
find /dev/input/ -maxdepth 1 -type c -exec chmod 666 '{}' \+
