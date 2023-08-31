[nrf52_nfc_module_doc]: https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fnfc.html

[nxp_rc522_datasheet]: https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf

# ChameleonUltra

Why not keep using ATXMEGA128?
First of all, it is difficult to buy chips because the lead time for the main chip is too long, and because the price has skyrocketed. Secondly, because the interaction speed of the ATXMEGA, simulation is slow, the decryption performance of the READER mode cannot meet the needs, and the LF support cannot be added, so we have been trying to upgrade it, such as using the latest ARM to replace the AVR framework, and the performance will definitely be greatly improved.

# Why nRF52840?

NRF52840 has a built-in NFC Tag-A module, but no one seems to care about it. After playing with HydraNFC's TRF7970A and FlipperZero's ST25R3916, the developers found that they can only simulate MIFARE Classic with a very high FDT.
We accidentally tested the NFC of nRF52840, and found that it is not only surprisingly easy to simulate a complete MIFARE Classic card, but also has very good simulation performance, friendly data flow interaction, and very fast response, unlike the former which is limited by the SPI bus clock rate. We also found that it has ultra-low power consumption, ultra-small size, 256kb/1M large RAM and Flash, also has BLE5.0 and USB2.0 FS, super CortexM4F, most importantly, it is very cheap! This is undoubtedly a treasure discovery for us!

Below we will explain in detail how we exploited the performance of the NRF52840, and what seemingly impossible functions have been realized with it!

# Supported functions

## High Frequency Attack

| Attack Type  |   Tag Type    | Whether the hardware supports | Does the software support | Whether the application layer supports |                    Note |
|--------------|:-------------:|------------------------------:|---------------------------|:--------------------------------------:|------------------------:|
| Sniffing     |      No       |                            No | No                        |                   No                   |                         |
| MFKEY32 V2   | MIFARE Classic |                       Support | Support                   |                Support                 | MIFARE Classic Detection |
| Darkside     | MIFARE Classic |                       Support | Support                   |                Support                 |    Encrypted 4 bit NAck |
| Nested       | MIFARE Classic |                       Support | Support                   |                Support                 |    PRNG(Distance guess) |
| StaticNested | MIFARE Classic |                       Support | Support                   |          Not yet implemented           |  PRNG(2NT Fast Decrypt) |
| HardNested   | MIFARE Classic |                       Support | Support                   |          Not yet implemented           |                      No |
| Relay attack |   ISO14443A   |                       Support | Support                   |          Not yet implemented           |                      No |

## High Frequency Simulation

| Card Type                     |    Encoding Type     | Whether the hardware supports | Does the software support | Whether the application layer supports |                                     Note |
|-------------------------------|:--------------------:|------------------------------:|---------------------------|:--------------------------------------:|-----------------------------------------:|
| Other than ISO14443A  |          No          |                            No | No                        |                   No                   | [NRF52 NFC Module][nrf52_nfc_module_doc] |
| NTAG 21x (210-218)            | ISO14443A/106 kbit/s |                       Support | Support                   |          Not yet implemented           |                                          |
| MIFARE Ultralight             | ISO14443A/106 kbit/s |                       Support | Support                   |          Not yet implemented           |                                          |
| MIFARE Ultralight Ev1         | ISO14443A/106 kbit/s |                       Support | Support                   |          Not yet implemented           |                                          |
| MIFARE Ultralight C           | ISO14443A/106 kbit/s |                       Support | Support                   |          Not yet implemented           |                                          |
| MIFARE Classic1K/2K/4K (4B/7B) | ISO14443A/106 kbit/s |                       Support | Support                   |                Support                 |                                          |
| MIFARE DESFire                | ISO14443A High Rate  |       Only supported Low rate | Only supported Low rate   |          Not yet implemented           |                                          |
| MIFARE DESFire EV1            | ISO14443A High rate  |       Only supported Low rate | Only supported Low rate   |          Not yet implemented           |                      Backward compatible |
| MIFARE DESFire EV2            | ISO14443A High rate  |       Only supported Low rate | Only supported Low rate   |          Not yet implemented           |                                          |
| MIFARE Plus                   | ISO14443A High rate  |       Only supported Low rate | Only supported Low rate   |          Not yet implemented           |                                          |

## High Frequency Reader

| Card Type                     |    Encoding Type     |                Whether the hardware supports | Does the software support                    | Whether the application layer supports |                                       Note |
|-------------------------------|:--------------------:|---------------------------------------------:|----------------------------------------------|:--------------------------------------:|-------------------------------------------:|
| Non <13.56MHz or ISO14443A>   |          No          |                                           No | No                                           |                   No                   | [NXP RC522 Datasheet][nxp_rc522_datasheet] |
| NTAG 21x (210-218)            | ISO14443A/106 kbit/s |                                      Support | Support                                      |          Not yet implemented           |                                            |
| MIFARE Ultralight             | ISO14443A/106 kbit/s |                                      Support | Support                                      |          Not yet implemented           |                                            |
| MIFARE Ultralight Ev1         | ISO14443A/106 kbit/s |                                      Support | Support                                      |          Not yet implemented           |                                            |
| MIFARE Ultralight C           | ISO14443A/106 kbit/s |                                      Support | Support                                      |          Not yet implemented           |                                            |
| MIFARE Classic 1K/2K/4K (4B/7B) | ISO14443A/106 kbit/s |                                      Support | Support                                      |                Support                 |                                            |
| MIFARE DESFire                | ISO14443A High Rate  | Supports low rates, or possibly higher rates | Supports low rates, or possibly higher rates |          Not yet implemented           |                                            |
| MIFARE DESFire EV1            | ISO14443A High rate  | Supports low rates, or possibly higher rates | Supports low rates, or possibly higher rates |          Not yet implemented           |                        Backward compatible |
| MIFARE DESFire EV2            | ISO14443A High rate  | Supports low rates, or possibly higher rates | Supports low rates, or possibly higher rates |          Not yet implemented           |                                            |
| MIFARE Plus                   | ISO14443A High rate  | Supports low rates, or possibly higher rates | Supports low rates, or possibly higher rates |          Not yet implemented           |                                            |

## Low Frequency Attack

| Vulnerability Type | Tag Type  | Whether the hardware supports | Does the software support | Whether the application layer supports | Note |
|--------------------|:---------:|------------------------------:|---------------------------|:--------------------------------------:|-----:|
| Sniffing           |  125KHz   |                       Support | Support                   |          Not yet implemented           |      |
| Brute Force        | EM410x ID |                       Support | Support                   |          Not yet implemented           |      |

## Low Frequency Simulation

| Card Type                | Encoding Type | Whether the hardware supports | Does the software support | Whether the application layer supports |                                          Note |
|--------------------------|:-------------:|------------------------------:|---------------------------|:--------------------------------------:|----------------------------------------------:|
| Other than <125KHz/ASK/PSK/FSK> |      No       |                            No | No                        |                   No                   | Only 125 khz RF, Modulation ASK, FSK and PSK. |
| EM410x                   |      ASK      |                       Support | Support                   |                Support                 |                   EM4100 is support(AD 64bit) |
| T5577                    |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| EM4306                    |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| HID Prox                 |      FSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Indala                   |      PSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| FDX-B                    |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Paradox                  |      FSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Keri                     |      PSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| AWD                      |      FSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| ioProx                   |      FSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| securakey                |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| gallagher                |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| PAC/Stanley              |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Presco                   |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Visa2000                 |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Viking                   |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Noralsy                  |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| NexWatch                 |      PSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Jablotron                |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |

## Low Frequency Reader

| Card Type                | Encoding Type | Whether the hardware supports | Does the software support | Whether the application layer supports |                                          Note |
|--------------------------|:-------------:|------------------------------:|---------------------------|:--------------------------------------:|----------------------------------------------:|
| Other than <125KHz/ASK/PSK/FSK> |      No       |                            No | No                        |                   No                   | Only 125 khz RF, Modulation ASK, FSK and PSK. |
| EM410x                   |      ASK      |                       Support | Support                   |                Support                 |                                               |
| T5577                    |      ASK      |                       Support | Support                   |             Support(Write)             |                                               |
| EM4305                    |      ASK      |                       Support | Support                   |             Not yet implemented             |                                               |
| HID Prox                 |      FSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Indala                   |      PSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| FDX-B                    |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Paradox                  |      FSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Keri                     |      PSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| AWD                      |      FSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| ioProx                   |      FSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| securakey                |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| gallagher                |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| PAC/Stanley              |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Presco                   |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Visa2000                 |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Viking                   |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Noralsy                  |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |
| NexWatch                 |      PSK      |                       Support | Support                   |          Not yet implemented           |                                               |
| Jablotron                |      ASK      |                       Support | Support                   |          Not yet implemented           |                                               |

## Low Frequency Modulation


| Modulation Type |                         wav |
|-----------------|----------------------------:|
| PSK             | ![PSK WAV](images/measured-psk.png) |
| FSK             | ![FSK WAV](images/measured-fsk.png) |
| ASK             | ![ASK WAV](images/measured-ask.png) |

# Ultra-low power consumption

It integrates a high-performance and low-power NFC module inside. When the NFC unit is turned on, the total current of the chip is only 5mA@3.3V.
The underlying interaction is done independently by the NFC unit and does not occupy the CPU.
In addition, the nRF52840 itself is a high-performance low-power BLE chip, and the encryption and calculation process is only 7mA@3.3V. It can greatly reduce the battery volume and prolong the working time. That is to say, the 35mAh 10mm*40mm button lithium battery can guarantee to be charged once every half a year under the working condition of swiping the card 8 times a day for 3 seconds each time. Full potential for everyday use.

# Not just UID, but a real and complete MIFARE Classic emulation

We can easily and completely emulate all data and password verification of all sectors, and can customize SAK, ATQA, ATS, etc. Similar to an open CPU card development platform, 14A interaction of various architectures can be easily realized.

# Super compatibility with low-power locks using batteries

The structure of the old Chameleon AVR is slow to start during simulation. Faced with a battery-powered low-power lock and an integrated lock on the door, it will be frequently interrupted, and the verification interaction cannot be completed completely, resulting in no response when swiping the card.

In order to reduce power consumption, the battery lock will send out a field signal as short as possible when searching for a card, which is no problem for the original card, but it is fatal for the MCU emulated card. Cards or mobile smart bracelets simulated by the MCU cannot wake up and respond in such a short time, so many battery locks cannot open the door, which greatly reduces the user experience.

This project specially optimizes the start-up and interaction logic and antenna for low-power reading heads. After testing a variety of common low-power reading heads, they can open the door perfectly by swiping the card.

# Ultra-fast response speed and low interaction delay(MIFARE Classic)

| Tag/Emulation            |             FDT             |                                "**_FDT_**" Rating                                |
|----------------------|:---------------------------:|:--------------------------------------------------------------------------------:|
| Standard MIFARE Card |  ![Standard_m1_s50](images/fdt_standard_s50.png)   | &#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50; |
| Chameleon Ultra       | ![Chameleon Ultra](images/fdt_chameleon_ultra.png) |         &#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;         |
| Proxmark3 Rdv4.01    |   ![Proxmark3_Rdv4_RRG_(Firmware build at 20201026)](images/fdt_pm3_rdv401.png)    |                         &#x2B50;&#x2B50;&#x2B50;&#x2B50;                         |
| RedMi K30            |    ![Xiaomi_k30u_smartkey](images/fdt_redmi_k30.png)    |                 &#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;                 |
| Chameleon Tiny        | ![Chameleon Tiny](images/fdt_chameleon_tiny.png)  |                     &#x2B50;&#x2B50;&#x2B50;&#x2B50;&#x2B50;                     |
| Flipper Zero          |  ![Flipper Zero](images/fdt_flipper_zero.png)   |                                 &#x2B50;&#x2B50;                                 |

# 256kB super large RAM cooperates with RC522 to enable attacks


| Attack Type  |              CLI               |
|--------------|:------------------------------:|
| MFKEY32 V2   | ![attack_MIFARE_mfkey32](images/cli_mfkey32v2.png)  |
| Darkside     | ![attack_MIFARE_darkside](images/cli_darkside.png) |
| Nested       |  ![attack_MIFARE_nested](images/cli_nested.png)  |
| StaticNested |          Coming Soon           |
| HardNested   |          Coming Soon           |
| Relay attack |          Coming Soon           |

# Hardware frame diagram

![hw frame diagram](images/hardware_overview.png)
