```python
import network
import urequests
import ujson
from machine import UART
import time

SSID = "fivefive"
PWD = "rererere"
API_KEY = "2660cc50780d82c3ade0ccca85b4e8ec"

CITIES = {
    0: "Taipei",
    1: "Banqiao",
    2: "Taoyuan",
    3: "Taichung",
    4: "Tainan",
    5: "Kaohsiung"
}

uart = UART(2, baudrate=9600, tx=17, rx=16)

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print("Connecting to WiFi...")
        wlan.connect(SSID, PWD)
        while not wlan.isconnected():
            time.sleep(0.2)
    print("WiFi Connected: {}".format(wlan.ifconfig()))

def get_temperature(city):
    try:
        url = "http://api.openweathermap.org/data/2.5/weather?q={},TW&units=metric&appid={}".format(city, API_KEY)
        res = urequests.get(url)
        data = ujson.loads(res.text)
        res.close()
        temp = int(round(data["main"]["temp"]))
        print("{} = {}°C".format(city, temp))
        return temp
    except Exception as e:
        print("Error: {}".format(e))
        return 255

def main_loop():
    while True:
        if uart.any():
            city_code = uart.read(1)
            if city_code:
                city_index = city_code[0]
                print("Received city index: {}".format(city_index))
                if 0 <= city_index <= 5:
                    city_name = CITIES[city_index]
                    temp = get_temperature(city_name)
                    if 0 <= temp <= 99:
                        uart.write(bytes([temp]))
                    else:
                        uart.write(bytes([255]))
                else:
                    uart.write(bytes([255]))
        time.sleep(0.1)

connect_wifi()
main_loop()

```
