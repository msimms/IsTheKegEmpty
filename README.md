# IsTheKegEmpty

## Build the Scale

### Print the Scale's Enclosure

The CAD file is provided in the repository. It should be printed using a material strong enough to hold the weight of the keg.

### Mount the load cells and load cell amplifiers

Load Cells: https://www.adafruit.com/product/4543
HX711 Load Cell Amps: https://www.amazon.com/dp/B07SGPX7ZH

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