# ESP32 Smart Bell Automation System

An ESP32-based automated bell scheduling system designed for schools.

## Features

- Scheduled bell ringing
- Manual override
- Multiple bell units synchronized via ESP-NOW
- Embedded microcontroller control

## Hardware

- ESP32
- Relay module
- Siren / bell
- RTC Module
- Power supply(12V)

## How it Works

The main ESP32 controller schedules bell events.  
Secondary nodes receive signals via ESP-NOW and trigger bells simultaneously.

## Future Improvements

- Local web interface
- Schedule editing
- Voice command integration
