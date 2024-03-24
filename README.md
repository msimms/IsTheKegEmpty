# IsTheKegEmpty

## Build the Scale

### Print the Scale's Enclosure

The CAD file is provided in the repository. It should be printed using a material strong enough to hold the weight of the keg.

### Configure the Scale Firmware Build

```
rustup target add thumbv6m-none-eabi
```

### Deploy the Scale Firmware

* Attach the Raspberry Pico Pi W, holding down the reset button while connecting to the computer's USB.
* Build the firmware. This will also deploy it to the microcontroller.

```
cargo run --bin scale
```

## Deploy the Web App


## Build the Mobile App
