# IoTsmartband
project for ICS 190

In this project, I build a simple IoT device-- a smartband.
I use Esp8266, UV sensor, PPG sensor, air quality sensor, accelemeter sensor
to get the steps, temp, UV rate and some other useful data. 

I use the architecture that send the data that the sensors get to the cloud, let
cloud process the data and send the processed data to the android app.

The project mainly contains three parts of the code
-- The arduino part that take advanages of all the sensors and send the data gathered to the cloud
-- The eclipse part. I write a simple java servlet to handle the request from arduino and take in params added in the urls and store those data to the local file
-- The android part, connect to the eclipse server to get data
