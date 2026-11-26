# OpticsFoundry Sequencer for Z-turn Board V2

## Introduction

The OpticsFoundry control system uses a Zynq 7020 based System on Module to create commands sent over parallel, SPI, and I2C bus to analog and digital in/outputs and direct digital synthesizers (AD9854, AD9858, AD9958). This repository contains the Vivado and Vitis projects needed to recreate the OpticsFoundry sequencer firmware for the Z-turn Board V2. [MicroZed 7020](https://github.com/opticsfoundry/OpticsFoundry_Sequencer_MZ) and [PYNQ Z2](https://github.com/opticsfoundry/OF_Sequencer_PYNQ) versions are also available. See [OpticsFoundry_ControlLight](https://github.com/opticsfoundry/OpticsFoundry_ControlLight) for the API and demo code, and [OpticsFoundry_Control](https://github.com/opticsfoundry/OpticsFoundry_Control) for a full fledged experiment control system, including GUI.

This project was inspired by [JQI AutomatioN for Experiments (JANE)](https://github.com/JQIamo/jane), see alse [Rev. Sci. Instrum. 92, 055107 (2021)](https://pubs.aip.org/aip/rsi/article-abstract/92/5/055107/1021868/Programmable-system-on-chip-for-controlling-an).

## Cloning

Clone this repository into the path
C:\OpticsFoundry\

For Vitis, it's easier to use this specific path than trying to adjust the project to a different one.

## To create the Z-turn V2 firmware

With Vitis 2023.1 (and not another Vitis version), open the folder 

C:\OpticsFoundry\OF_Sequencer_Zturn\OF_Sequencer_Zturn_Vitis_2023.1

An empty workspace should appear. To import the project(s), select

File -> Import -> Import projects from Git -> Existing local repository -> OF_Sequencer_Zturn -> Import existing Eclipse projects -> OF_Sequencer_Zturn_Vitis_2023.1 -> Finish

Right click "OpticsFoundry_Seq...App" (not "..App_system")

Select "Properties"

Select "C/C++ Build -> Settings"

Select "ARM v7 gcc linker -> Inferred Options -> Software Platform"

Put the following under "Software Platform Inferred Flags":

-Wl,--start-group,-lxil,-lgcc,-lc,-lm,--end-group

-Wl,--start-group,-lxil,-llwip4,-lgcc,-lc,-lm,--end-group

To recreate and flash the firmware do the following:

Right click on "Assistant" OpticsFoundry_Seq_Zturn_App -> Debug and select it.

Right click on projects in "Explorer" and select "Build all", or just "Build", then "Create Boot Image".

Right click on "Assistant" OpticsFoundry_Seq_Zturn_App -> Release and select it.

Right click on projects in "Explorer" and select "Build all", or just "Build", then "Create Boot Image".

Connect the Z-turn V2 to power using a Mini USB cable. Connect a suitable JTAG programmer to your PC, e.g. a Digilent JTAG-HS2 using a Micro USB cable. Plug both USB cables into the same computer. Connect the JTAG programmer to the Z-turn Board V2 using an adapter cable or board. If you use the adapter cable delivered with your control system: the side with the cable marked in red points towards the edge of the Z-turn board and goes into the VDD pin of the Digilent JTAG-HS2. To enter JTAG programming mode, you need to set JP1 of the Z-turn board to Closed and JP2 to Open before connecting the board to power, see Photo 1 below. 

After powering on the Z-turn and the JTAG programmer, select "Explorer" -> "Program Flash". If the JTAG cable is not detected, you might have to download the lated Digilent driver. After you finish programming, you need to set both jumpers to Closed, see Photo 2 below.

Use [OpticsFoundry_ControlLight](https://github.com/opticsfoundry/OpticsFoundry_ControlLight) to test the sequencer.

![Programming the Z-turn V2](Photos/programming_Z-turn_V2.jpeg)
**Photo 1:** Programming the Z-turn V2; pay attention to put JP2 to Open before powering up the board and to the programming cable orientation.

![Using the Z-turn V2](Photos/using_Z-turn_V2.jpeg)
**Photo 2:** Using the Z-turn V2; pay attention to setting JP2 to Closed.

## Updating the firmware over Ethernet

Firmware built from this repository from 2026-11-26 on can write a new boot image into its own QSPI flash over Ethernet. No JTAG programmer is needed. (A board running older firmware needs to be updated once using JTAG, see above, or an SD card, see below.)

1. In Vitis, build the firmware and select "Create Boot Image", as described above. This creates

   C:\OpticsFoundry\OF_Sequencer_Zturn\OF_Sequencer_Zturn_Vitis_2023.1\OpticsFoundry_Seq_Zturn_App_system\_ide\bootimage\BOOT.bin

2. In [OpticsFoundry_ControlLight](https://github.com/opticsfoundry/OpticsFoundry_ControlLight), run `DemoUpdateSequencerFirmware()` in ControlLight.cpp. It calls `CLA_UpdateSequencerFirmware(0, "...\\BOOT.bin")` and then `CLA_RebootSequencer(0)`. In Python, use `update_sequencer_firmware(0, path)` and `reboot_sequencer(0)`.

3. The update of a 4.4 MB image takes about 25 s (erase 14 s, program 8 s, read-back verification 1.5 s). Don't switch the board off during that time. After the reboot, the board is reachable again after about 10 s.

`DemoSequencerFirmwareInfo()` (`CLA_GetSequencerFirmwareInfo`, Python `get_sequencer_firmware_info`) shows the build date and time of the running firmware and whether it was booted from QSPI flash or SD card.

Before erasing anything, the firmware receives the complete image and checks its size, its CRC32 checksum and its Zynq boot header. It refuses to update while a sequence is running. After programming, it reads the flash back and compares it to the image. The running firmware executes from DDR memory, so it can rewrite the QSPI flash it was booted from. The Ethernet commands (`get_firmware_info`, `update_firmware_qspi`, `reboot_system`) are described in OpticsFoundry_Seq_Zturn_App/src/firmware_update.h.

### Booting from SD card

To recover from a failed update (e.g. a power failure during programming), to update a board running older firmware without a JTAG programmer, or to test firmware without overwriting the QSPI flash, boot the Z-turn V2 from a microSD card:

1. Format a microSD card with FAT32 and copy BOOT.bin onto it, named BOOT.BIN.
2. Switch the board off, set JP1 to Open and JP2 to Closed (SD card boot mode), insert the card, and switch the board on. The boot mode jumpers are only read at power-on.
3. If needed, use the Ethernet update described above to write the firmware into the QSPI flash.
4. Switch the board off, set both jumpers to Closed (QSPI boot mode, Photo 2), and switch it on again.


## Modifying the C code of the Z-turn V2 firmware

This code is based on the Vitis example "lwIP echo server". Most of the sequencer specific code is contained in firefly.c.

To change the IP configuration (MAC address, DHCP or static IP) follow the instructions in

C:\OpticsFoundry\OF_Sequencer_Zturn\OF_Sequencer_Zturn_Vitis_2023.1\OpticsFoundry_Seq_Zturn_App\src\main.c.

Project specific TCP/IP communication code is contained in echo.c. An interpreter for a very simple programing language running on the ARM core of the MircroZed is implemented using the code in the OpticsFoundryCPUCommandSequencer folder.


## To create the FPGA bitstream

Use Vivado 2023.1 (and not another Vivado version) to open project 

C:\OpticsFoundry\OF_Sequencer_Zturn\OF_Sequencer_Zturn_Vivado_2023.1\FireflyControl_1.xpr

Update all IP.

Under "Flow Navigator", click on "Generate Bitstream".

Ignore the two "BD 41-237" warnings.

After creating the bitstream (which takes about 15 minutes), export it using

File -> Export -> Export Hardware -> "Include Bitstream"-> Finish

To include this bitstream in the Z-turn Board V2 firmware, in Vitis, in "Explorer" right click on 

OpticsFoundry_Seq_Zturn_Platform 

select "Update Hardware Specifications" and select the file you just exported, i.e. usually

C:/OpticsFoundry/OF_Sequencer_Zturn/OF_Sequencer_Zturn_Vivado_2023.1/design_1_wrapper.xsa

Then, as explained above, rebuild the firmware, create boot image, and program flash.

## Modifying the FPGA design

Open the project and then "Open Block Design". After the project has fully loaded, you can access the source files in the Block Design window under "Sources -> Design Sources". 

The main part of the code is contained in the file core.sv (under core_wrapper -> core_inst: core). It shouldn't be too difficult to understand the design by reading this file. It might be informative to read it to understand the sequences that OpticsFoundry_Control and OpticsFoundry_ControlLight send to the sequencer. (You can inspect an ASCII file dump of those sequences by enabling the "Debug FPGA buffer" option in the "System debug options" menu of OpticsFoundry_Control).

The pinout to the Z-turn Board V2 header is defined under "Constraints -> constr_1 -> MicroZed_constraints.xdc".

