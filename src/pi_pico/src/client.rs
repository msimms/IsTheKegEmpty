#![no_std]
#![no_main]

use defmt_rtt as _;
use embassy_executor::Spawner;
use embassy_net::ipv4::{Config, StackResources, Stack, Interface};
use embassy_net::smoltcp::phy::DeviceCapabilities;
use embassy_net::smoltcp::time::Instant;
use embassy_net::smoltcp::wire::{EthernetAddress, IpAddress, IpCidr, Ipv4Address};
use embassy_net_driver::ethernet::EthernetDevice;
use embassy_rp::eth::Ethernet;
use embassy_rp::Peripherals;
use embassy_time::{Duration, Timer};
use embedded_hal::digital::v2::OutputPin;
use embedded_svc::io::Write;
use embedded_svc::io;
use {defmt, panic_probe as _};
use surf;

#[embassy_executor::main]
async fn start_client(spawner: Spawner) {
    defmt::info!("Hello, world!");

    // Initialize the network interface
    let p = embassy_rp::init(Default::default());
    let eth = Ethernet::new(p.ETH);

    let config = Config::static_ip(Ipv4Address::new(192, 168, 1, 100), Ipv4Address::new(255, 255, 255, 0), Ipv4Address::new(192, 168, 1, 1), None);

    let mut resources = StackResources::<1, 1, 8>::new();
    let stack = Stack::new(eth, config, &mut resources);

    // Spawn a network task
    spawner.spawn(network_task(stack)).unwrap();

    // Delay to ensure network stack is up
    Timer::after(Duration::from_secs(5)).await;

    // Make a GET request
    match surf::get("http://example.com").recv_string().await {
        Ok(response) => defmt::info!("Response: {}", response),
        Err(e) => defmt::error!("Error: {:?}", e),
    }
}

#[embassy_executor::task]
async fn network_task(stack: Stack<EthernetDevice>) {
    loop {
        stack.poll().await;
    }
}
