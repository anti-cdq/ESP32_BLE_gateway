# ESP32 Full Test

Brief Introduction be edit later.

# Hardware

* 240*240 TFT LCD
* 8* buttons
    * RESET(CHIP_PU)
    * UP
    * DOWN
    * LEFT
    * RIGHT
    * MIDDLE
    * BACK
    * BOOT
* 1* LED (GPIO21)
* Micro SD card
* 2* DAC (GPIO25&GPIO26, connect to PAM8403 to drive two speakers)
* QMC5883L (3-axis Magnetic Sensor)(IIC_SCL/GPIO17, IIC_SDA/)
* MPU6050 (6-axis motion tracking devices)
* UART -> CP2104 -> Micro USB (TXD0/GPIO1, RXD0/GPIO3)
* Power supply voltage detect (SENSOR_VN/IO39)
* Power switch between USB or battery

# Software
Current include tasks:

* Main task to init hardware, manage LCD display, create task, delete task
* Button detect task
* BLE Scan task
* Wifi scan task
* File I/O via SD card