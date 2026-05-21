NERV🍁 ~/Public/ydotool/build > cat /etc/udev/rules.d/80-uinput.rules
KERNEL=="uinput", GROUP="input", MODE="0660", OPTIONS+="static_node=uinput"

$ sudo rm /etc/udev/rules.d/80-uinput.rules
$ sudo udevadm control --reload-rules && sudo udevadm trigger

$ sudo gpasswd -d $USER input
