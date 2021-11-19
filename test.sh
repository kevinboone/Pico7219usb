DEV=/dev/ttyACM0

function send_and_receive ()
  {
  echo "$1" > $DEV
  read RESP < $DEV
  # TODO -- check error response
  }

# Enable hardware flow control and disable echo. These 
#  settings are essential for reliable communication. 
stty -F /dev/ttyACM0 speed 115200 > /dev/null
stty -F /dev/ttyACM0 crtscts -icanon -iexten -echo

# Reset the display
send_and_receive "R"

# Send the text "hello"
send_and_receive "Dhello   "

# Turn scrolling on
send_and_receive "G"

sleep 4

# Turn scrolling off 
send_and_receive "H"

