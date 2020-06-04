# Low Power SPI with FreeRTOS

In this project, the i.MX RT 1050 EVK board is used (More information about the board [here](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/i-mx-rt1050-evaluation-kit:MIMXRT1050-EVK)),
and is communicating with the HMC5983 through SPI (Note that the HMC5983 will use I2C for SOC-i, but there was no
other device I had that use's SPI so I chose HMC5983). This project also have an example of creating multiple task and queues, and demonstrate how you could block a task until it is triggered. In this project, hopefully you'll get a better understanding 
understanding on how to use the MCUXpresso IDE and the basic of FreeRTOS. 


## Getting Started

To start, you'll need to instal the IDE, drivers, toolchains, and terminal emulator. Follow the setup [here](https://www.nxp.com/document/guide/get-started-with-the-mimxrt1050-evk:GS-MIMXRT1050-EVK) 
to get the necessary softwares. 

When you want to use the LPSPI of the i.MX RT 1050 EVKB, you'll need to remove and add some resistors on the board (Not sure why, it was 
mentioned in the LPSPI example). The resistors you'll need to remove is R334 and you'll need to add 0 ohms resistor to R278, R279, R280, 
and R281. The resistors R278 - R281 are all located on the underside of the board, below the SD card slot. The R334 is located on the top side 
near the 2 micro USB port. 

## Examples 

There are some examples provided in MCUXpresso which could be useful to you. You can import the example in MXUXpresso's quickstart panel,as seen in the red boxed area Fig. 1. There's a lot of examples which could be overwhelming, but SOC-i will not be using all the features available so pick an example that is most relevant, and see if there's an example of it in FreeRTOS. 

![quickpanel](https://github.com/AA-CubeSat-Team/Sensor/blob/master/FreeRTOS/evkbimxrt1050_freertos_lpspi/images/quickpanel.jpg "Quick Panel")

## What's in this project
Once you download this project and place it in to your workspace, the main function (like many of the examples) will be located under the directory source, and in a file called freertos_lpspi.c. In the C file, it use's SPI to read the magnetometer data, sets up tasks, and queuing items. Since the actual cubeSAT will have multiple of the same sensors, we emulated having these sensors connected. For now, it just queue up dummy data, but hopefully we can implement reading multiple sensors soon.

## Edit SPI configuration

You might want to edit some properties of the SPI such as the baud rate, clock polarity, pins, etc. You could do that by going to one of the perspectives MXUXpresso's provide. Those perspectives are located in the top right, as seen in figure below. 

![MCXpresso](https://github.com/AA-CubeSat-Team/Sensor/blob/master/FreeRTOS/evkbimxrt1050_freertos_lpspi/images/MCUXpresso_main.png "MCUXpresso Main Page")

To see a list of available perspective, you could click on the icon ![perspective][perspective]. We will only look at a few of the perspective, but feel free to explore around (and maybe add them to this text and describe them). The ![dev][develop] is the developer's perspective which is the default view. In that perspective, you could make edit to the source code directly. The ![debug][debug] is the debugging perspective, which is the perspective you'll get when you're running your software while debugging. The ![pins][pin] is the pin perspective, where you could define which pins of the CPU will be used for. For instance, you could change the pins associated to the SPI. In ![peripheral][peripheral] is the peripheral perspective, where you could define and set the configuration of different kind of peripherlal that is available on the board. In ![clock][clock] is the clock perspective, which allows you to change the clock signal of different component of the board. 

## 

## Useful Links





[perspective]: https://github.com/AA-CubeSat-Team/Sensor/blob/master/FreeRTOS/evkbimxrt1050_freertos_lpspi/images/perspective.jpg#thumbnail "Perspective"
[pins]: https://github.com/AA-CubeSat-Team/Sensor/blob/master/FreeRTOS/evkbimxrt1050_freertos_lpspi/images/pins.jpg "Pins"
[peripheral]: https://github.com/AA-CubeSat-Team/Sensor/blob/master/FreeRTOS/evkbimxrt1050_freertos_lpspi/images/pins.jpg "Peripheral"
[clock]: https://github.com/AA-CubeSat-Team/Sensor/blob/master/FreeRTOS/evkbimxrt1050_freertos_lpspi/images/clocks.jpg "Clocks"
[debug]: https://github.com/AA-CubeSat-Team/Sensor/blob/master/FreeRTOS/evkbimxrt1050_freertos_lpspi/images/debug.jpg "Debug"
[develop]: https://github.com/AA-CubeSat-Team/Sensor/blob/master/FreeRTOS/evkbimxrt1050_freertos_lpspi/images/develop.jpg "Develop"


