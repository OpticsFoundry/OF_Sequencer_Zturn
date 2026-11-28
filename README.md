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

### Installing the firmware on a new board with an SD card (no JTAG programmer needed)

A board with any other firmware in its QSPI flash (e.g. a factory fresh Z-turn) can be set up with an installer SD card:

1. Format a microSD card with FAT32 (FAT16 also works). Copy .\OF_Sequencer_Zturn_Vitis_2023.1\OpticsFoundry_Seq_Zturn_App_system\_ide\bootimage\BOOT.bin onto it, named BOOT.BIN, and create an empty file named INSTALL (any extension, e.g. INSTALL.txt) next to it, both in the root folder. To copy the firmware of a working board instead, put the card into the PC's card reader and call `CLA_CopySequencerFirmwareToSDCard(Sequencer, "E:\\")` (Python: `copy_sequencer_firmware_to_sd_card`, demo: `DemoCopySequencerFirmwareToSDCard()` in ControlLight.cpp). It reads the firmware from the board's QSPI flash over Ethernet (the image it booted from, after an SD card boot its primary image) and writes BOOT.BIN and INSTALL onto the card. `CLA_SaveSequencerFirmware(Sequencer, "file.bin")` saves that image as a file. Both need firmware from 2026-11-26 (evening) on, which has the Ethernet command `read_firmware_image`.

2. Switch the board off, set JP1 to Open and JP2 to Closed (SD card boot mode), insert the card, and switch the board on. Make sure both jumpers are seated properly: with both jumpers open, the board doesn't boot at all.

3. The firmware copies BOOT.BIN into both QSPI image slots (primary and backup) and verifies them. This takes about 50 s (about 25 s per slot; slots that already contain the image are skipped). Meanwhile the status LED (LED1 on the front panel of the carrier board, and the Z-turn RGB LED) blinks blue, toggling with each 64 KB block written or checked (since 2026-11-27; before, it was steadily blue), then green with two short beeps when the firmware was installed, or one short beep if the QSPI flash already contained exactly this image. Red and three long beeps mean an error; the USB UART output (115200 baud) and the firmware info (`DemoSequencerFirmwareInfo()`) give details. Don't switch the board off while the LED blinks blue. (If that happens anyway, simply boot from the installer card again.)

4. Switch the board off, set both jumpers to Closed (QSPI boot mode), remove the SD card, and switch it on again.

The carrier board's LED1 is an inverted copy of the Z-turn's onboard RGB LED. The firmware drives the LED such that LED1 on the front panel shows the intended colour, so the onboard LED shows the complementary colour (`STATUS_LED_ACTIVE_LOW` in FireflyControl.c).

Without the INSTALL file, booting from the SD card works as described below and doesn't change the QSPI flash. The Ethernet command `install_from_sd` installs BOOT.BIN from an SD card inserted into a running board, independent of the boot mode and of the INSTALL file (see sd_installer.h).

## Updating the firmware over Ethernet

Firmware built from this repository from 2026-11-26 on can write a new boot image into its own QSPI flash over Ethernet. No JTAG programmer is needed. (A board running older firmware needs to be updated once using JTAG, see above, or an SD card, see below.)

1. In Vitis, build the firmware and select "Create Boot Image", as described above. This creates

   C:\OpticsFoundry\OF_Sequencer_Zturn\OF_Sequencer_Zturn_Vitis_2023.1\OpticsFoundry_Seq_Zturn_App_system\_ide\bootimage\BOOT.bin

   This file is only replaced by "Create Boot Image" (or the equivalent command line `D:\Xilinx\Vitis\2023.1\bin\bootgen.bat -arch zynq -image ...\_ide\bootimage\OpticsFoundry_Seq_Zturn_App_system.bif -w -o i ...\_ide\bootimage\BOOT.bin`). Building the application alone leaves an older BOOT.bin there. If in doubt, check the file date, or search the file for the build date string (e.g. "Nov 28 2026 14:15:50"), which `get_firmware_info` reports after installation.

2. In [OpticsFoundry_ControlLight](https://github.com/opticsfoundry/OpticsFoundry_ControlLight), run `DemoUpdateSequencerFirmware()` in ControlLight.cpp. It calls `CLA_UpdateSequencerFirmware(0, "...\\BOOT.bin")` and then `CLA_RebootSequencerWithTrialBoot(0)` (see "Trial boot" below; `CLA_RebootSequencer(0)` reboots without trial). In Python, use `update_sequencer_firmware(0, path)` and `reboot_sequencer_with_trial_boot(0)` (or `reboot_sequencer(0)`).

3. The update of a 4.4 MB image takes about 25 s (erase 14 s, program 8 s, read-back verification 1.5 s). Don't switch the board off during that time. After the reboot, the board is reachable again after about 10 s.

`DemoSequencerFirmwareInfo()` (`CLA_GetSequencerFirmwareInfo`, Python `get_sequencer_firmware_info`) shows the build date and time of the running firmware, whether it was booted from QSPI flash or SD card, from which flash image it was booted, and whether the flash contains valid primary and backup images.

Before erasing anything, the firmware receives the complete image and checks its size, its CRC32 checksum and its Zynq boot header. It refuses to update while a sequence is running. After programming, it reads the flash back and compares it to the image. The running firmware executes from DDR memory, so it can rewrite the QSPI flash it was booted from. The Ethernet commands (`get_firmware_info`, `update_firmware_qspi`, `update_backup_firmware_qspi`, `test_backup_boot`, `reboot_system`) are described in OpticsFoundry_Seq_Zturn_App/src/firmware_update.h, `trial_reboot` and `confirm_firmware` in trial_boot.h.

### Backup image

The 16 MB QSPI flash holds two boot images:

| QSPI offset | Content |
|---|---|
| 0x000000 | primary image, written by `CLA_UpdateSequencerFirmware` and by Vitis "Program Flash" |
| 0x800000 | backup image, written by `CLA_UpdateSequencerBackupFirmware` (Python `update_sequencer_backup_firmware`) |
| 0xFF0000 | settings: network settings (0xFF0000, 0xFF1000), trial boot record (0xFF2000) |

If the primary image has no valid boot header, the Zynq BootROM boots the backup image. Images are written with their boot header last, so an update that is interrupted, e.g. by a power failure, leaves the primary image without a valid boot header, and the board boots the backup image. FSBL also falls back to the backup image if it can't load the partitions of the primary image. The firmware info then reports "image BACKUP". Write the primary image again to recover.

Write a well tested firmware as backup image, e.g. by setting `AlsoUpdateBackupImage` in `DemoUpdateSequencerFirmware()` once the new firmware has proven itself. The sequencer refuses to write the backup image while the primary image is invalid. The Ethernet command `test_backup_boot` erases the boot header of the primary image and reboots, to check that the backup image boots; afterwards write the primary image again.

By itself, the backup image doesn't help if a new primary image boots, but doesn't work (e.g. hangs or has no network connection). Trial boot (below) covers that case. Without it, boot from SD card or use JTAG. Vitis "Program Flash" only erases the flash sectors it writes, so it keeps the backup image (unless "erase all" is selected).

### Trial boot

Since 2026-11-27 a new primary image can be booted "on trial": `CLA_RebootSequencerWithTrialBoot(0)` (Python `reboot_sequencer_with_trial_boot(0)`), called after `CLA_UpdateSequencerFirmware`, reboots the sequencer with a watchdog running, waits until the new firmware answers (about 8 s), checks that it runs the image just written, and confirms it. The confirmation stops the watchdog; from then on the firmware runs without watchdog, as before. If the new firmware isn't confirmed within 120 s (it hangs, has no network, crashes, or the PC is gone), the watchdog resets the board and it boots the backup image. The backup firmware then erases the boot header of the failed primary image, so that the board keeps booting the backup image also after power cycles, and `get_firmware_info` reports "trial boot: FAILED ...". Write a new primary image to recover. A failed trial takes about 50 s with a 30 s timeout (tested).

Requirements: the running firmware and the new image both support trial boot (built from 2026-11-27 on; the sequencer checks the new image for marker strings in its FSBL and application and refuses otherwise), and there is a valid backup image that is not the same as the new image. For a complete fallback, the backup firmware should support trial boot, too; an older backup firmware still boots, but then the board tries the failed primary image again (with trial) after each power cycle. Write the new firmware as backup image only after it has been confirmed (`AlsoUpdateBackupImage` in `DemoUpdateSequencerFirmware()` does that).

How it works: the running firmware stores a "pending" trial record (size and CRC32 of the new image, timeout) at QSPI offset 0xFF2000 and reboots. The FSBL (OpticsFoundry_Seq_Zturn_Platform/zynq_fsbl/of_trial_boot.c, called from main.c) finds the record, checks the CRC32 of the primary image and the boot header of the backup image, sets a bit in the REBOOT_STATUS register and starts the Cortex-A9 private watchdog (AWDT0, which resets the whole system; no change of the Vivado design is needed). REBOOT_STATUS survives the watchdog reset, so after an unconfirmed trial the FSBL boots the backup image instead. A deliberate reboot during the trial, or a power cycle, starts a new trial. While a firmware runs on trial and isn't confirmed, firmware updates are refused. Details: OpticsFoundry_Seq_Zturn_App/src/trial_boot.h (Ethernet commands `trial_reboot`, `confirm_firmware`).

The FSBL is part of the platform project. Vitis copies the built fsbl.elf to OpticsFoundry_Seq_Zturn_Platform/export/.../boot/fsbl.elf, which "Create Boot Image" uses: after changing the FSBL, build the platform (or run make in zynq_fsbl and copy fsbl.elf there). If Vitis ever regenerates the FSBL sources, add of_trial_boot.c/.h and the call of `OfTrialBoot_Check()` in main.c again.

### Installing the firmware using a JTAG programmer

Connect the Z-turn V2 to power using a Mini USB cable. Connect a suitable JTAG programmer to your PC, e.g. a Digilent JTAG-HS2 using a Micro USB cable. Plug both USB cables into the same computer. Connect the JTAG programmer to the Z-turn Board V2 using an adapter cable or board. If you use the adapter cable delivered with your control system: the side with the cable marked in red points towards the edge of the Z-turn board and goes into the VDD pin of the Digilent JTAG-HS2. To enter JTAG programming mode, you need to set JP1 of the Z-turn board to Closed and JP2 to Open before connecting the board to power, see Photo 1 below. 

After powering on the Z-turn and the JTAG programmer, select "Explorer" -> "Program Flash". If the JTAG cable is not detected, you might have to download the lated Digilent driver. After you finish programming, you need to set both jumpers to Closed, see Photo 2 below.

Use [OpticsFoundry_ControlLight](https://github.com/opticsfoundry/OpticsFoundry_ControlLight) to test the sequencer.

![Programming the Z-turn V2](Photos/programming_Z-turn_V2.jpeg)
**Photo 1:** Programming the Z-turn V2; pay attention to put JP2 to Open before powering up the board and to the programming cable orientation.

![Using the Z-turn V2](Photos/using_Z-turn_V2.jpeg)
**Photo 2:** Using the Z-turn V2; pay attention to setting JP2 to Closed.

### Booting from SD card

To recover from a failed update (e.g. a power failure during programming), to update a board running older firmware without a JTAG programmer, or to test firmware without overwriting the QSPI flash, boot the Z-turn V2 from a microSD card:

1. Format a microSD card with FAT32 and copy BOOT.bin onto it, named BOOT.BIN. If a file named INSTALL is present on the SD card, the SD card's BOOT.BIN will be installed as new firmware in the QSPI flash. If you don't want that, make sure there is no file named INSTALL on the SD card.
2. Switch the board off, set JP1 to Open and JP2 to Closed (SD card boot mode), insert the card, and switch the board on. The boot mode jumpers are only read at power-on.
3. To boot again from QSPI, switch the board off, set both jumpers to Closed (QSPI boot mode, Photo 2), and switch it on again.

## MAC address and finding sequencers on the network

Firmware from 2026-11-26 on gives each board its own MAC address 02:4F:46:xx:xx:xx ("OF", a locally administered address). xx:xx:xx are the lower 24 bits of the CRC32 of the factory programmed 64 bit unique ID of the Winbond QSPI flash. So several sequencers can be used on one network without changing the firmware. (If the flash has no unique ID, the previous fixed MAC address 00:0A:35:F1:01:03 is used.) When a board changes from older firmware to this one, its MAC address and usually also its IP address change. The MAC address and the TCP port 57978 can't be changed.

To find the sequencers and their IP addresses, e.g. for ControlHardwareConfig.json, run `DemoDiscoverSequencers()` in ControlLight.cpp (`CLA_DiscoverSequencers`, Python `discover_sequencers()`). It sends the UDP datagram "OpticsFoundry_discover" to port 57979 as broadcast on all network interfaces. Each sequencer answers with one line containing its MAC address, IP address, netmask, gateway, IP source (DHCP, fixed or link-local), TCP port, firmware build and boot status. Sequencers with an IP address of another subnet answer by broadcast, so they can be found, too.

To tell several boards apart, hold the USR button of one Z-turn board while discovering: that board answers "USR button PRESSED" (since 2026-11-28). Don't hold it while switching the board on, which resets its network settings to DHCP. The 4 DIP switches SW1 of the Z-turn could serve as a board ID, but they are connected to PL pins (R19, T19, G14, J15, switch on = low), which the current FPGA design uses for leftover MicroZed ports (`trigger_out`, `JX1_SE_1`, `ext_condition_1`, PL UART RX); reading them needs a change of the Vivado design (see ToDo).

## IP address: DHCP or fixed (since 2026-11-27)

- **DHCP** (factory setting): the board gets its IP address from a DHCP server. If none answers within 3 s, it uses a link-local address 169.254.x.y (netmask 255.255.0.0; x and y are derived from the MAC address, so the address is always the same for one board), as a PC without DHCP server does. A PC connected directly by cable then finds it without configuration, provided the PC's network adapter is also set to DHCP (Windows then also uses a 169.254.x.y address). DHCP keeps running in the background and replaces the link-local address when a server appears; if a DHCP lease is lost, the board returns to its link-local address after 3 s. (Before 2026-11-27 the fallback was 192.168.1.10.)
- **Fixed IP**: stored IP address, netmask and optional gateway. Useful if the PC's network adapter has a fixed address, e.g. PC 192.168.1.1 and sequencer 192.168.1.10.

Set it with `CLA_SetSequencerFixedIP` / `CLA_SetSequencerDHCP` (Python `set_sequencer_fixed_ip`, `set_sequencer_dhcp`), read it with `CLA_GetSequencerNetworkSettings`; see `DemoSequencerNetworkSettings()` in ControlLight.cpp. The sequencer stores the setting in the QSPI flash (settings sector 0xFF0000, two copies, so a power failure while writing keeps the old setting) and reboots. Then enter its new IP address in ControlHardwareConfig.json.

If the sequencer can't be reached any more (e.g. a fixed IP address in the wrong subnet):
- Discovery still finds it. Then `CLA_SetSequencerDHCPByMAC` or `CLA_SetSequencerFixedIPByMAC` (Python `set_sequencer_dhcp_by_mac`, `set_sequencer_fixed_ip_by_mac`) sets it by its MAC address with a UDP broadcast, which works across subnets and without ControlHardwareConfig.json. The sequencer reboots afterwards.
- Or hold the Z-turn USR button (S1, MIO50) while switching the sequencer on, until it beeps twice (about 2 s after power-on; the RGB LED is yellow while the button is held, then green). This sets it back to DHCP. Discovery then reports "network settings reset by USR button at start-up".

Firmware protocol: TCP commands `get_network_settings`, `set_network_settings`, UDP command `OpticsFoundry_set_network <MAC> dhcp|fixed <IP> <netmask> [<gateway>]` to port 57979; see network_settings.h. `get_firmware_info` also shows the current state of the USR button.

Only one client can be connected to a sequencer at a time. A new TCP connection replaces the previous one, which is closed.


## Modifying the C code of the Z-turn V2 firmware

This code is based on the Vitis example "lwIP echo server". Most of the sequencer specific code is contained in firefly.c.

The IP configuration (DHCP, fixed IP, link-local fallback) is set at run time, see above and network_settings.c; the MAC address is computed in sequencer_identity.c. The BSP must keep LWIP_DHCP enabled.

Project specific TCP/IP communication code is contained in echo.c. An interpreter for a very simple programing language running on the ARM core of the MircroZed is implemented using the code in the OpticsFoundryCPUCommandSequencer folder.

Network servicing while the firmware is busy (since 2026-11-27): a CPU command sequence started with an Ethernet check period of n > 0 ms services the network every n ms; with a period <= 0 (fast mode) every 100 ms (`FAST_MODE_NETWORK_SERVICE_PERIOD_MS` in FireflyControl.h), so that it can always be stopped with `cs_stop_sequence`. `wait_till_finished` with a timeout <= 1 ms also services the network every 100 ms. Before, a period >= 0 serviced the network after every command (about 10% slower), and the fast modes never did. The interpreter command `CheckEthernetCommand(n)` services the network if at least n ms have passed since the last time (n <= 0: always).

Interrupting a wait (since 2026-11-27): while the firmware waits for the end of a sequence (`wait_till_finished`, `wait_till_end_of_sequence_then_get_input_data`), it doesn't dispatch commands. The PC can interrupt the wait with the urgent commands `interrupt_wait` (the sequence continues) and `interrupt_wait_and_stop` (the sequence is stopped; an interrupted `wait_till_end_of_sequence_then_get_input_data` always stops it). echo.c handles them as soon as they are received and doesn't queue them, so they never get an answer; the interrupted wait answers `2` instead of `1` (finished) or `0` (timeout). They only act while a wait command is running or pending, so a late interrupt can't hit the next wait. The PC API sends them as `interrupt_wait0`, which older firmware ignores without answering. On the PC: `CLA_InterruptWaitTillEndOfSequence(StopSequence)`, called from another thread; see `DemoInterruptWaitTillEndOfSequence()` in ControlLight.cpp. `cs_interrupt_sequence` now also ends a `WaitTillSequenceFinished` in a CPU command sequence (the FPGA sequence continues).

TCP timing (since 2026-11-27): the lwIP timer interrupt runs every 2 ms (platform_zynq.c), so that delayed ACKs go out within 2 ms. tcp_slowtmr (retransmissions, connection timeouts) runs every 50 ms, 10x faster than lwIP's nominal 500 ms: a lost segment is retransmitted after about 100-150 ms, and a connection is only given up after 10-20 s without ACK. The DHCP timers run at their nominal periods. Before, tcp_slowtmr and the DHCP timers ran every 4 ms (128x too fast), so a connection was given up after about 1 s without ACK from the PC. Answers are sent immediately (tcp_output in server_send_ascii, Nagle algorithm off); before, they waited for the next lwIP poll in tcp_slowtmr. A command round trip from Python now takes about 0.2 ms (median; maximum about 1 ms), before about 2 ms (maximum 4 ms). Incoming commands never depended on the timer: RunEventLoop polls the Ethernet controller on every pass.

The development test commands `test1`, `test2`, `test_timer` and `adc` are disabled (`ENABLE_TEST_COMMANDS` in FireflyControl.c), because they overwrite the loaded sequence, switch the clock or block the firmware. When enabled for a development build, they are ignored while a sequence is running.

The build date and time reported by `get_firmware_info` are those of the build: makefile.defs in the application folder (included by the makefile Vitis generates) makes every build recompile firmware_update.c.


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
- (done 2026-11-27: fixed IP address or DHCP with link-local fallback, reset by UDP command or USR button at power-on; MAC address and port deliberately not configurable)
- Second reset button on the front panel, on an unused Z-turn header pin (needs a bitstream change, or an MIO pin).
- Board ID from the DIP switches SW1 (PL pins R19, T19, G14, J15; 10K pull-up, switch on = low): add a 4 bit AXI GPIO input in the block design, move these pins from the leftover MicroZed ports to it, and report the pattern in the discovery answer. Needs a new bitstream and a hardware update of the Vitis platform (check that the FSBL trial boot changes survive it). Be careful: before using these pins for the switches, check that they are truly not used by anything else, in particular on the OpticsFoundry carrier board (schematic of the carrier board and of the Z-turn, and every place in the FPGA design and firmware that uses `trigger_out`, `JX1_SE_1`, `ext_condition_1` or the PL UART).
- (done 2026-11-27: "trial boot": a watchdog plus confirmation of a new primary image by the PC, falling back to the backup image if a new image boots but doesn't work; see "Trial boot" above)
- Authentication of the PC, at least for firmware updates (signed images, or pairing with a button press).

