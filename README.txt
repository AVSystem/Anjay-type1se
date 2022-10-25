Anjay client with FOTA for L462 with Murata TypeSC1 modem

1. Some important notes:
 - Requires STM32CubeIDE (tested on version 1.9.0)
 - Python prebuild/posbuild scripts require numpy and pycryptodomex libs
 - To build the whole firmware, you need to build first the SECoreBin project, then SBSFU project, and in the end the AnjayType1SE project which is the acual Anjay app. Yeah, it also makes one of the leds blink.
 - The important build arifacts are stored in Projects/B-L462E-CELL1/Applications/2_Images_ExtFlash/AnjayType1SE/Binary directory and they are:
   * SBSFU_AnjayType1SE.bin - which contains the whole firmware mapping with SBSFU and Anjay application
   * AnjayType1SE.sfb - which contains only the Anjay application and this is the file which should be send using the LwM2M server for the update

