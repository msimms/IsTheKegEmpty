# IsTheKegEmpty

This is a work-in-progress. Just a simple system for measuring the contents of my homebrew keg, by weight, and reporting it to a simple web server via a REST API.

## Build the Scale

### Print the Scale Enclosure

The CAD file is provided in the repository. It should be printed using a material strong enough to hold the weight of the keg.

### Scale Hardware

Load Cells: https://www.adafruit.com/product/4543

HX711 Load Cell Amps: https://www.amazon.com/dp/B07SGPX7ZH

### Mount the load cells and load cell amplifiers

Attach the load cell wires to the terminal block
TODO - add pic

## Build the Scale Firmware (Arduino Nano 33 IoT with Headers)

### Hardware

https://www.amazon.com/gp/product/B07WPFQZQ1

### Add support for the board to the Arduino IDE:

Tools -> Board -> Booard Manager -> Install Arduino SAMD Boards

### Add supporting libraries to the Arduino IDE:

Tools -> Manager Libraries
Install the following libraries:
* Adafruit SSD1306 (for the screen)
* HX711 Arduno Library by Bogdan Necula
* WiFiNINA

### Build and Upload the Firmware

## Build the Scale Firmware (Raspberry Pi Pico W)

```
Note: This is a Second firmware option and is still under development
```

### Hardware
https://www.amazon.com/Pico-Raspberry-Pre-Soldered-Dual-core-Processor/dp/B0BK9W4H2Q

### Configure the Scale Firmware Build

```
rustup target add thumbv6m-none-eabi
```

### Deploy the Scale Firmware

* Attach the Raspberry Pico Pi W, holding down the reset button while connecting to the computer's USB.
* Build the firmware. This will also deploy it to the microcontroller.

```
cd src/sensor # If not already in this directory
cargo run --bin scale
```

## Deploy the Web App

### Install Supporting Packages

```
cd src/web # If not already in this directory
python3 setup.py
```

## Build the Mobile App

* Open XCode
* Build
