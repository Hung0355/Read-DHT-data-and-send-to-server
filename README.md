# Read-DHT-data-and-send-to-server
Read temperature and humidity from DHT11 and send them to server via MQTT 
- NODE ESP32
  + Get data from DHT11.
  + Display data on LCD SSD1306 (I2C)
  + Send to server period 10s.
- SERVER
  + Get data from MQTT.
  + Save data to MySQL.
  + Build web app.
- END-USER
  + Create chart using Chart.js
