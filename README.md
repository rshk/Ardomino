# Ardomino

Talking sensor framework: a paradigmatic solution to communicate information
of local measured data.


## Trying out

Simple instructions to try the Ardomino Sketch.


### Dependency libraries

You need to install the following libraries:

* [DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
* [WiFly Library](https://github.com/harlequin-tech/WiFlyHQ)

To do so:

```shell
mkdir -p ~/Arduino/libraries
cd ~/Arduino/libraries
git clone https://github.com/adafruit/DHT-sensor-library DHT_sensor_library
git clone https://github.com/harlequin-tech/WiFlyHQ WiFlyHQ
```

..then of course remember to restart the Arduino IDE.


### Local settings

Before building, go in the ``ardomino`` folder and create a ``settings.h``
to define connection parameters:

```c++
// ESSID and passphrase of the WiFi network
#define WIFI_SSID "ArdominoHQ"
#define WIFI_PASSWORD "ArdominoRocks"

// Address of the TCP server to which to send data
#define SERVER_ADDR "10.99.66.1"
#define SERVER_PORT 43555
```

### Dummy receiver server

To test that everything works fine, you can launch a testing server with:

```shell
while :; do nc -l -p 43555 -vvv; done
```

(nc is from ``netcat-openbsd``, the "traditional" one has slightly different
arguments).


### The hardware

..then of course, you need an Ardomino. I got one, what about you? :P
