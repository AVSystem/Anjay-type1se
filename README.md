# Anjay Type1SE FOTA demo

Anjay client with FOTA for Murata TypeSE1 module that can be found on B-L462E-CELL1 development kit.

This example is built upon Secure Boot and Secure Firmware Update Demo Application provided by STMicroelectronics

Copyright (c) 2017 STMicroelectronics.<br/>
Copyright (c) 2022 AVSystem.<br/>
All rights reserved.

This software is licensed under terms that can be found in the LICENSE file in
the root directory of this software component. <br/>
If no LICENSE file comes with this software, it is provided AS-IS.

## Secure Boot and Secure Firmware Update Demo Application

The X-CUBE-SBSFU Secure Boot and Secure Firmware Update solution allows the update of the STM32 microcontroller built-in
program with new firmware versions, adding new features and correcting potential issues. The update process is performed
in a secure way to prevent unauthorized updates and access to confidential on-device data such as secret code and
firmware encryption key.

The Secure Boot (Root of Trust services) is an immutable code, always executed after a system reset, that checks STM32
static protections, activates STM32 runtime protections and then verifies the authenticity and integrity of user
application code before every execution in order to ensure that invalid or malicious code cannot be run.

This examples is provided with a dual firmware image configuration, with second image based on external flash in Type1SE module,
in order to ensure safe image installation and enable over-the-air firmware update capability commonly used in IOT devices.

This example is based on a third party cryptographic middleware : MbedTLS.
Cryptographic services are delivered as open source code. It can be configured to use asymmetric or symmetric
cryptographic schemes with or without firmware encryption.

For more details, refer to UM2262 "Getting started with SBSFU - software expansion for STM32Cube" available from the
STMicroelectronics microcontroller website www.st.com.
## Directory contents
Project files are placed under `Projects/B-L462E-CELL1/Applications/2_Images_ExtFlash`

- **2_Images_SECoreBin**<br/>
  Generate secure engine binary including all the "trusted" code
- **2_Images_SBSFU**<br/>
  Secure boot and secure firmware update application
- **AnjayType1SE**<br/>
  Anjay application with Firmware Update object
- **Linker_Common**<br/>
  Linker files definition shared between SECoreBin, SBSFU, AnjayType1SE

## How to use it ?

You need to follow a strict compilation order :

1. Compile SECoreBin application<br/>
   This step is needed to create the Secure Engine core binary including all the "trusted" code and keys mapped inside
   the protected environment. The binary is linked with the SBSFU code in step 2.
2. Compile SBSFU application<br/>
   This step compiles the SBSFU source code implementing the state machine and configuring the protections. In addition,
   it links the code with the SECore binary generated at step 1 in order to generate a single SBSFU binary including the
   SE trusted code.
3. Compile UserApp application<br/>
   Remember to change your application settings in `Projects/B-L462E-CELL1/Applications/2_Images_ExtFlash/AnjayType1SE/Stack/App/anjay_client_config.h` file.<br/>
   It generates:<br/>
   - The user application binary file that is uploaded to the device using the Secure Firmware Update process <br/>
     (`Projects/B-L462E-CELL1/Applications/2_Images_ExtFlash/AnjayType1SE/Binary/AnjayType1SE.sfb`).
   - A binary file concatenating the SBSFU binary, the user application binary in clear format, and the corresponding
     FW header. <br/>
     (`Projects/B-L462E-CELL1/Applications/2_Images_ExtFlash/AnjayType1SE/Binary/SBSFU_AnjayType1SE.bin`).
4. Flashing<br/>
   Use STM32CubeProgrammer application with `SBSFU_AnjayType1SE.bin` file to program the board. You can open serial port to check if Anjay has successfully connected
   to the server using configuration provided in step 3.<br/>
   After that, you can use Coiote DM to perform firmware update with `AnjayType1SE.sfb` file.


## Connecting to the LwM2M Server
To connect to [Coiote IoT Device Management](https://www.avsystem.com/products/coiote-iot-device-management-platform/) LwM2M Server, please register at [https://eu.iot.avsystem.cloud/](https://eu.iot.avsystem.cloud/). There is a [guide showing basic usage of Coiote DM](https://iotdevzone.avsystem.com/docs/Coiote_DM_Device_Onboarding/Quick_start/)
available on IoT Developer Zone.
## Links
* [Anjay source repository](https://github.com/AVSystem/Anjay)
* [Anjay documentation](https://avsystem.github.io/Anjay-doc/index.html)
* [Doxygen-generated API documentation](https://avsystem.github.io/Anjay-doc/api/index.html)
* [AVSystem IoT Devzone](https://iotdevzone.avsystem.com/)
* [AVSystem Discord server](https://discord.avsystem.com)
