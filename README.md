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

`DemoSequencerFirmwareInfo()` (`CLA_GetSequencerFirmwareInfo`, Python `get_sequencer_firmware_info`) shows the build date and time of the running firmware, whether it was booted from QSPI flash or SD card, from which flash image it was booted, and whether the flash contains valid primary and backup images.

Before erasing anything, the firmware receives the complete image and checks its size, its CRC32 checksum and its Zynq boot header. It refuses to update while a sequence is running. After programming, it reads the flash back and compares it to the image. The running firmware executes from DDR memory, so it can rewrite the QSPI flash it was booted from. The Ethernet commands (`get_firmware_info`, `update_firmware_qspi`, `update_backup_firmware_qspi`, `test_backup_boot`, `reboot_system`) are described in OpticsFoundry_Seq_Zturn_App/src/firmware_update.h.

### Backup image

The 16 MB QSPI flash holds two boot images:

| QSPI offset | Content |
|---|---|
| 0x000000 | primary image, written by `CLA_UpdateSequencerFirmware` and by Vitis "Program Flash" |
| 0x800000 | backup image, written by `CLA_UpdateSequencerBackupFirmware` (Python `update_sequencer_backup_firmware`) |
| 0xFF0000 | reserved for settings |

If the primary image has no valid boot header, the Zynq BootROM boots the backup image. Images are written with their boot header last, so an update that is interrupted, e.g. by a power failure, leaves the primary image without a valid boot header, and the board boots the backup image. FSBL also falls back to the backup image if it can't load the partitions of the primary image. The firmware info then reports "image BACKUP". Write the primary image again to recover.

Write a well tested firmware as backup image, e.g. by setting `AlsoUpdateBackupImage` in `DemoUpdateSequencerFirmware()` once the new firmware has proven itself. The sequencer refuses to write the backup image while the primary image is invalid. The Ethernet command `test_backup_boot` erases the boot header of the primary image and reboots, to check that the backup image boots; afterwards write the primary image again.

The backup image doesn't help if a new primary image boots, but doesn't work (e.g. has no network connection). In that case, boot from SD card or use JTAG. Vitis "Program Flash" only erases the flash sectors it writes, so it keeps the backup image (unless "erase all" is selected).

### Booting from SD card

To recover from a failed update (e.g. a power failure during programming), to update a board running older firmware without a JTAG programmer, or to test firmware without overwriting the QSPI flash, boot the Z-turn V2 from a microSD card:

1. Format a microSD card with FAT32 and copy BOOT.bin onto it, named BOOT.BIN.
2. Switch the board off, set JP1 to Open and JP2 to Closed (SD card boot mode), insert the card, and switch the board on. The boot mode jumpers are only read at power-on.
3. If needed, use the Ethernet update described above to write the firmware into the QSPI flash.
4. Switch the board off, set both jumpers to Closed (QSPI boot mode, Photo 2), and switch it on again.

## MAC address and finding sequencers on the network

Firmware from 2026-11-26 on gives each board its own MAC address 02:4F:46:xx:xx:xx ("OF", a locally administered address). xx:xx:xx are the lower 24 bits of the CRC32 of the factory programmed 64 bit unique ID of the Winbond QSPI flash. So several sequencers can be used on one network without changing the firmware. (If the flash has no unique ID, the previous fixed MAC address 00:0A:35:F1:01:03 is used.) The board gets its IP address by DHCP; when a board changes from older firmware to this one, its MAC address and usually also its IP address change.

To find the sequencers and their IP addresses, e.g. for ControlHardwareConfig.json, run `DemoDiscoverSequencers()` in ControlLight.cpp (`CLA_DiscoverSequencers`, Python `discover_sequencers()`). It sends the UDP datagram "OpticsFoundry_discover" to port 57979 as broadcast on all network interfaces. Each sequencer answers with one line containing its MAC address, IP address, TCP port, firmware build and boot status. Sequencers with an IP address of another subnet answer by broadcast, so they can be found, too.

Only one client can be connected to a sequencer at a time. A new TCP connection replaces the previous one, which is closed.


## Modifying the C code of the Z-turn V2 firmware

This code is based on the Vitis example "lwIP echo server". Most of the sequencer specific code is contained in firefly.c.

To change the IP configuration (DHCP or static IP; the MAC address is computed in sequencer_identity.c) follow the instructions in

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

## ToDo

Network and maintenance:
- Command to set a fixed MAC address and/or a fixed IP address (or switch back to the defaults), stored in the settings sector of the QSPI flash (0xFF0000), with a way to reset it if the board becomes unreachable (e.g. a UDP command addressed by MAC address, or a button pressed at boot).
- "Trial boot": a watchdog plus confirmation of a new primary image by the PC, falling back to the backup image if a new image boots but doesn't work. Needs a modified FSBL.
- Authentication of the PC, at least for firmware updates (signed images, or pairing with a button press).

Found in the code review of 2026-11-26 and not yet changed (functional issues, not memory safety):
- `cs_execute_sequence` with a negative Ethernet check period doesn't service the network, so such a CPU command sequence can't be stopped. `wait_till_finished` with a timeout <= 1 ms also doesn't service the network (by design, for speed).
- CPU command sequencer: `PID` reads `args[7]` although it is declared with 7 arguments; `GetFPGABuffer` is declared with 1 argument but uses 2; the type checks of `SetFPGABuffer`, `SetAD9854*`, `SetAnalogOut` and `RunFastVCOLoop` use `||` where `&&` is meant; `Foo ` and `Foo` (trailing space) are different variable names.
- The test commands `test2` and `adc` overwrite the loaded sequence and flush the cache with a wrong address range.
- `ReadInputBuffer` uses `>` instead of `>=` in a bounds check; `FireflyControl_start` truncates a 64-bit start position to 32 bits.
- In `ExtIrq_Handler`, the computation of `TransferToDo` (`extended_addr / (DMA0TransferSize/4)`) looks inconsistent with the 32768-command BRAM halves and should be checked.

