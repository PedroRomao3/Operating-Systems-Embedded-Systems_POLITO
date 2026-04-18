# FreeRTOS Bare-Metal Application & Testing Framework

[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![RTOS](https://img.shields.io/badge/OS-FreeRTOS-green.svg)](https://www.freertos.org/)
[![Architecture](https://img.shields.io/badge/Arch-ARM_Cortex--M3-orange.svg)](https://developer.arm.com/ip-products/processors/cortex-m)

## 📌 Project Overview
This repository contains a bare-metal embedded application built on top of the **FreeRTOS** kernel. While the repository includes the standard FreeRTOS source and verification tools, **my core development and contribution reside entirely within the `freertos/Demo/` directory.**

The goal of this project was to configure a real-time operating system from scratch for an **ARM Cortex-M3** target (MPS2), develop bare-metal hardware drivers, and implement an automated testing pipeline to verify task scheduling and system behavior. 

This project demonstrates my ability to work closely with hardware, configure RTOS environments, and implement rigorous testing methodologies—skills critical for the automotive and microelectronics industries.

## 🚀 Key Features Developed (`Demo/` folder)

* **Bare-Metal ARM Cortex-M3 Initialization:** * Wrote the hardware startup code (`startup.c`) and custom linker scripts (`mps2_m3.ld`) to correctly initialize the memory layout, stack, and heap for the Cortex-M3 processor before handing control over to the RTOS.
* **Custom Driver Development:** * Implemented a bare-metal UART driver (`uart.c`, `uart.h`) for serial communication without relying on heavy external HAL libraries.
* **System Logging Architecture:** * Developed a structured logging system (`logging.c`, `logging_levels.h`) to trace task execution, RTOS state changes, and system events.
* **Automated "Golden Master" Testing Framework:** * Designed a comprehensive testing plan (`testing_plan.md`) to validate RTOS behavior.
  * Built a Bash-based automated test runner (`test_runner.sh`) that executes the compiled firmware, captures the serial output, and automatically compares it against expected "golden" reference files (`tests/golden/`). This ensures deterministic behavior and prevents regressions.
* **RTOS Configuration:** * Tailored the `FreeRTOSConfig.h` to optimize task scheduling, memory management, and tick rates for the specific hardware target.

## 🧰 Technology Stack

* **Language:** Embedded C 
* **Operating System:** FreeRTOS (Custom configured)
* **Target Architecture:** ARM Cortex-M3 (MPS2)
* **Toolchain:** ARM GCC (`arm-none-eabi-gcc`), GNU Make
* **Testing:** Bash scripting, Golden Master Testing (Output Verification)

## 📂 My Contribution Structure (`freertos/Demo/`)

    freertos/Demo/
    ├── startup.c / mps2_m3.ld   # Bare-metal startup and memory linker scripts
    ├── main.c / setup.c         # Application entry point and hardware setup
    ├── uart.c / uart.h          # Bare-metal UART peripheral drivers
    ├── logging.c / logging.h    # Custom diagnostic logging stack
    ├── FreeRTOSConfig.h         # RTOS behavioral configuration
    ├── testing_plan.md          # Documented test cases and expected outcomes
    ├── test_runner.sh           # Automated test execution script
    ├── tests/golden/            # Expected output files for verification
    └── output/                  # Actual logs generated during test execution

*(Note: Directories outside of `freertos/Demo/` contain the upstream FreeRTOS kernel, ports, and external verification tools provided by the FreeRTOS project).*

## 🛠️ Getting Started

### Prerequisites
* ARM GCC Toolchain (`arm-none-eabi-gcc`)
* QEMU (for ARM Cortex-M emulation, if running without physical MPS2 hardware)
* GNU Make

### Building and Testing
Navigate to the Demo directory to build the project and run the automated test suite:

    cd freertos/Demo
    
    # Build the firmware using Make
    make all
    
    # Run the automated testing pipeline
    ./test_runner.sh
