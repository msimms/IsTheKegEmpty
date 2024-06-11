#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]

use defmt_rtt as _;
use embassy_executor::Spawner;
use embassy_rp::gpio::{Level, Pull, Input, Output};
use embassy_time::{Duration, Timer};
use panic_probe as _;

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    defmt::info!("Initializing...");

    let peripherals = embassy_rp::init(Default::default());
    let data_pin = Input::new(peripherals.PIN_17, Pull::None);
    let mut clock_pin = Output::new(peripherals.PIN_3, Level::Low);
    let gain = 0;
    let mut value = 0;

    defmt::info!("Initialized.");

    loop {
        // Read the 24-bit value from the HX711
        for _ in 0..24 {
            clock_pin.set_high();
            Timer::after(Duration::from_micros(1)).await;
            value = (value << 1) | data_pin.is_high() as u32;
            clock_pin.set_low();
            Timer::after(Duration::from_micros(1)).await;
        }

        // Set the gain for the next reading
        for _ in 0..gain {
            clock_pin.set_high();
            Timer::after(Duration::from_micros(1)).await;
            clock_pin.set_low();
            Timer::after(Duration::from_micros(1)).await;
        }

        // Check if the value is negative and extend the sign
        if value & 0x800000 != 0 {
            value |= 0xFF000000;
        }
    }
}
