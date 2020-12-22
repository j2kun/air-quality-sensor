# Air Quality Sensor

A little Arduino project for displaying current AQI and an icon
to tell you whether it's healthy to breathe.

Shows a chart of recent AQI measurements over time.

Part List:
 - SparkFun BlackBoard C (Running in 3.3V mode)
 - SparkFun TMP102 digital temperature sensor
 - Zio Qwiic 1.5" OLED 128x128
 - Panasonic SN-GCJA5 Particle Sensor (red=5V, black=GND, yellow=SCL, blue=SDA)

Icons from https://www.iconpacks.net/free-icon-pack/emoticons-127.html

## Dependencies 

 - https://github.com/sparkfun/SparkFun_Particle_Sensor_SN-GCJA5_Arduino_Library
 - https://github.com/olikraus/u8g2
 - https://github.com/rlogiacco/CircularBuffer
