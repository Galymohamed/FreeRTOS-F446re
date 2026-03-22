# STM32 CMake Project Template v3.0.0

**Author:** Mohamed GALY
**Supported boards:** STM32F446RE (Nucleo-F446RE)
**Toolchain:** arm-none-eabi-gcc | **Generator:** Ninja | **CMake >= 3.25**

---

## Description

A structured CMake-based firmware template for the STM32F446RE (Nucleo-F446RE) running FreeRTOS. It provides a clean starting point for embedded applications with the following features:

- **FreeRTOS** — pre-integrated kernel with heap_4 and Cortex-M4F port
- **UART driver** — supports polling, interrupt, DMA, and circular-buffer (CB) modes
- **GPIO** — EXTI interrupt handling with named pin constants
- **Circular buffer** — generic byte-level circular buffer with ASCII/binary pattern matching
- **Delay utilities** — DWT-based microsecond delay and flag-wait helpers with timeout
- **BSP layer** — board-specific peripheral initialization cleanly separated from application code

---

## Requirements

- CMake >= 3.25
- Ninja
- arm-none-eabi-gcc toolchain
- OpenOCD (for flashing)

| Preset | Board | Build Type | Binary Dir |
|---|---|---|---|
| `f446-debug` | STM32F446RE | Debug (`-Og -g3`) | `build/f446/debug` |
| `f446-release` | STM32F446RE | Release (`-Os -g0`) | `build/f446/release` |
| `f446-relwithdebinfo` | STM32F446RE | RelWithDebInfo | `build/f446/relwithdebinfo` |


## Contact

For any questions or inquiries: **contact@galytek.com**
