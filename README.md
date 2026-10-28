# Safety Band GNSS

A simple Zephyr application for Nordic nRF91 series devices that starts a GNSS receiver and monitors satellite fix information using the modem GNSS API.

This project is designed for cellular IoT development boards such as the nRF9160 DK and related nRF91 platforms. It initializes the modem, enables GNSS operation, continuously searches for a fix, and logs latitude/longitude/altitude information when a valid position is available.

## Features

- Initializes the Nordic modem library
- Configures GNSS use case and operational mode
- Starts continuous GNSS scanning
- Logs satellite status and fix information
- Turns on an LED when a valid fix is obtained
- Works with Zephyr / Nordic Connect SDK (NCS) projects

## Project structure

- `CMakeLists.txt` - Zephyr build configuration
- `Kconfig` - GNSS timing and timeout configuration
- `prj.conf` - project configuration for the sample
- `sample.yaml` - sample metadata for supported hardware
- `src/main.c` - main GNSS application logic

## Hardware and requirements

This sample targets Nordic nRF91 devices, including:

- nRF9151 DK
- nRF9161 DK
- nRF9160 DK
- Thingy:91 / Thingy:91 X

You will also need:

- Zephyr SDK / Nordic Connect SDK environment
- `west` build tool
- A supported nRF91 development board

## Build

From the project root:

```bash
west build -p auto -b nrf9160dk/nrf9160/ns
```

Or use another supported target from `sample.yaml`.

## Flash to device

```bash
west flash
```

## How it works

`src/main.c` does the following:

1. Initializes the modem and GNSS event handler.
2. Configures the GNSS use case and LTE functional mode.
3. Starts continuous GNSS scanning.
4. Listens for `NRF_MODEM_GNSS_EVT_PVT` events.
5. Reads satellite and PVT data.
6. Logs fix quality and highlights whether a 2D or 3D fix was found.

## Configuration

The GNSS timing can be adjusted in `Kconfig`:

- `GNSS_PERIODIC_INTERVAL` - periodic fix interval in seconds
- `GNSS_PERIODIC_TIMEOUT` - maximum fix timeout

## Notes

- The sample uses a modem-based GNSS implementation, so it is intended for hardware that includes the required modem support.
- GPS fix detection depends on satellite visibility and RF conditions.
- LED behavior is used as a simple status indicator when a fix is achieved.

