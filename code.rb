require "rubygems"
require "arduino_firmata"
 
arduino = ArduinoFirmata.connect
puts "firmata version #{arduino.version}"
 
loop do
  puts ("Press enter to open the garage door.")
  answer = gets 
  arduino.digital_write 2, true
  arduino.digital_write 6, true
  sleep 0.1
  arduino.digital_write 2, false
  arduino.digital_write 6, false
end