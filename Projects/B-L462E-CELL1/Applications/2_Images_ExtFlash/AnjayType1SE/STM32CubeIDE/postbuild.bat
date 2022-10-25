::Post build for SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256
:: arg1 is the build directory
:: arg2 is the elf file path+name
:: arg3 is the bin file path+name
:: arg4 is the firmware Id (1/2/3)
:: arg5 is the version
:: arg6 when present forces "bigelf" generation
@echo off

set "projectdir=."
set "appname=STM32L476_SimpleApp
set "execname=%appname%"
set "elf=debug\%appname%.elf"
set "bin=debug\%appname%.bin"
set "version=1"


pushd ..\%appname%

echo Generating secure binaries executing in %cd%
Rem set "SBSFUBootLoader=%~d0%~p0\\..\\.."
set "SBSFUBootLoader=%cd%\..\STM32CubeExpansion_SBSFU_V2.4.0\Projects\NUCLEO-L476RG\Applications\2_Images

::The default installation path of the Cube Programmer tool is: "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin"
::If you installed it in another location, please update the %programmertool% variable below accordingly.
set "programmertool="C:\\Program Files\\STMicroelectronics\\STM32Cube\\STM32CubeProgrammer\\bin\\STM32_Programmer_CLI""
set "userAppBinary=%projectdir%\\Binary\\"

set "sfu=%userAppBinary%\%execname%.sfu"
set "sfb=%userAppBinary%\%execname%V%version%.sfb"
set "sign=%userAppBinary%\%execname%.sign"
set "headerbin=%userAppBinary%\%execname%sfuh.bin"
set "bigbinary=%userAppBinary%\SBSFU_%execname%V%version%.bin"
set "elfbackup=%userAppBinary%\SBSFU_%execname%.elf"
set "partialbin=%userAppBinary%\\Partial%execname%.bin"
set "partialsfb=%userAppBinary%\\Partial%execname%.sfb"
set "partialsfu=%userAppBinary%\\Partial%execname%.sfu"
set "partialsign=%userAppBinary%\\Partial%execname%.sign"
set "partialoffset=%userAppBinary%\\Partial%execname%.offset"
set "ref_userapp=%projectdir%\\RefUserApp.bin"

set "iv=%SBSFUBootLoader%\2_Images_SECoreBin\Binary\iv.bin"
set "ecckey=%SBSFUBootLoader%\2_Images_SECoreBin\Binary\ECCKEY1.txt"
set "sbsfuelf=%SBSFUBootLoader%\2_Images_SBSFU\SW4STM32\STM32L476RG_NUCLEO_2_Images_SBSFU\Debug\SBSFU.elf"
set "oemkey=%SBSFUBootLoader%\2_Images_SECoreBin\Binary\OEM_KEY_COMPANY1_key_AES_CBC.bin"
set "magic=SFU1"

::comment this line to force python
::python is used if windows executeable not found
::pushd %projectdir%\..\..\..\..\..\..\Middlewares\ST\STM32_Secure_Engine\Utilities\KeysAndImages
pushd ..\STM32CubeExpansion_SBSFU_V2.4.0\Middlewares\ST\STM32_Secure_Engine\Utilities\KeysAndImages

set basedir=%cd%
popd
goto exe:
goto py:
:exe
::line for window executeable
echo Postbuild with windows executable
set "prepareimage=%basedir%\win\prepareimage\prepareimage.exe"
set "python="
if exist %prepareimage% (
goto postbuild
)
:py
::line for python
echo Postbuild with python script
set "prepareimage=%basedir%\prepareimage.py"
set "python=python "
:postbuild

::Make sure we have a Binary sub-folder in UserApp folder
if not exist "%userAppBinary%" (
mkdir "%userAppBinary%"
)

::PostBuild is fired if elf has been regenerated from last run, so elf is different from backup elf
if exist %elfbackup% (
fc %elfbackup% %elf% >NUL && (goto nothingtodo) || echo "elf has been modified, processing required"
)

echo Encrypt %bin% into %sfu% using AES key from SecureEngine binary
set "command=%python%%prepareimage% enc -k %oemkey% -i %iv% %bin% %sfu%  > %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error

echo Compute signature (sha256) of %bin% in %sign%
set "command=%python%%prepareimage% sha256 %bin% %sign% >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error

echo Generate update firmware : Header (version, iv, sha256 and own signature) + encrypted firmware in %sfb%
set "command=%python%%prepareimage% pack -m %magic% -k %ecckey%  -r 28 -v %version% -i %iv% -f %sfu% -t %sign% %sfb% -o 512 >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error

echo Generate header to be included in big image in %headerbin%
set "command=%python%%prepareimage% header -m %magic% -k  %ecckey% -r 28 -v %version%  -i %iv% -f %sfu% -t %sign% -o 512 %headerbin% >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error

echo Generate big image composed of SBSFU + Header + appp firmware in clear in %bigbinary%
set "command=%python%%prepareimage% merge -v 0  -i %headerbin% -s %sbsfuelf% -u %elf% %bigbinary% >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error

::Partial image generation if reference userapp exists
if not exist "%ref_userapp%" (
goto :bigelf
)
set "command=%python%%prepareimage% diff -1 %ref_userapp% -2 %bin% %partialbin% -a 16 --poffset %partialoffset% >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error

set "command=%python%%prepareimage% enc -k %oemkey% -i %iv% %partialbin% %partialsfu%  >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error

set "command=%python%%prepareimage% sha256  %partialbin% %partialsign% >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error

set "command=%python%%prepareimage% pack -m %magic% -k %ecckey%  -r 28 -v %version% -i %iv% -f %sfu% -t %sign% -o 512 --pfw %partialsfu% --ptag %partialsign% --poffset  %partialoffset% %partialsfb%>> %projectdir%\output.txt 2>&1"
%command%

IF %ERRORLEVEL% NEQ 0 goto :error

:bigelf
IF  "%~6"=="" goto :finish
echo "Generating the global elf file (SBSFU and userApp)"
echo "Generating the global elf file (SBSFU and userApp)" >> %projectdir%\output.txt
set "command=%programmertool% -ms %elf% %headerbin% %sbsfuelf% >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error
set "command=copy %elf% %elfbackup%  >> %projectdir%\output.txt 2>&1"
%command%
IF %ERRORLEVEL% NEQ 0 goto :error
:finish
::backup and clean up the intermediate file
del %sign%
del %sfu%
del %headerbin%
if exist "%ref_userapp%" (
del %partialsign%
del %partialbin%
del %partialsfu%
del %partialoffset%
)
popd
pause
goto :eof
::exit 0

:error
echo "%command% : failed" >> %projectdir%\output.txt
:: remove the elf to force the regeneration
if exist %elf%(
  del %elf%
)
if exist %elfbackup%(
  del %elfbackup%
)
echo "%command%" : failed
popd
pause
::exit 1

:nothingtodo
::exit 0

:eof