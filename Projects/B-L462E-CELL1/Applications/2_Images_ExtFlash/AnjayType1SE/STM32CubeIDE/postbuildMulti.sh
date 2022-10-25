#!/bin/bash -
#Post build for SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256
# arg1 is the build directory
# arg2 is the elf file path+name
# arg3 is the bin file path+name
# arg4 is the firmware Id (1/2/3)
# arg5 is the version
# arg6 when present forces "bigelf" generation
set -x
projectdir=..
execname=AnjayType1SE
elf=$execname.elf
bin=$execname.bin
fwid=1
version=1

SecureEngine=../../../2_Images_ExtFlash/2_Images_SECoreBin/STM32CubeIDE

userAppBinary=$projectdir"/../Binary"

sfu=$userAppBinary"/"$execname".sfu"
sfb=$userAppBinary"/"$execname".sfb"
sign=$userAppBinary"/"$execname".sign"
headerbin=$userAppBinary"/"$execname"sfuh.bin"
headerbin1=$userAppBinary"/"$execname"sfuh1.bin"
headerbin2=$userAppBinary"/"$execname"sfuh2.bin"
headerbin3=$userAppBinary"/"$execname"sfuh3.bin"
bigbinary=$userAppBinary"/SBSFU_"$execname".bin"
bigbinary1=$userAppBinary"/SBSFU_"$execname"1.bin"
bigbinary2=$userAppBinary"/SBSFU_"$execname"2.bin"
bigbinary3=$userAppBinary"/SBSFU_"$execname"3.bin"

iv=$SecureEngine"/../Binary/iv.bin"
magic="SFU"$fwid
oemkey=$SecureEngine"/../Binary/OEM_KEY_COMPANY"$fwid"_key_AES_CBC.bin"
ecckey=$SecureEngine"/../Binary/ECCKEY"$fwid".txt"
partialbin=$userAppBinary"/Partial"$execname".bin"
partialsfb=$userAppBinary"/Partial"$execname".sfb"
partialsfu=$userAppBinary"/Partial"$execname".sfu"
partialsign=$userAppBinary"/Partial"$execname".sign"
partialoffset=$userAppBinary"/Partial"$execname".offset"
ref_userapp=$projectdir"/RefUserApp.bin"
offset=512
alignment=16

current_directory=`pwd`
cd "$SecureEngine/../../"
SecureDir=`pwd`
cd "$current_directory"
sbsfuelf="$SecureDir/2_Images_SBSFU/STM32CubeIDE/Debug/SBSFU.elf"

current_directory=`pwd`
cd "../../../../../../Middlewares/ST/STM32_Secure_Engine/Utilities/KeysAndImages"
basedir=`pwd`
cd "$current_directory"
# test if window executable usable
prepareimage=$basedir"/win/prepareimage/prepareimage.exe"
uname | grep -i -e windows -e mingw > /dev/null 2>&1

if [ $? -eq 0 ] && [   -e "$prepareimage" ]; then
  echo "prepareimage with windows executable"
  export PATH=$basedir"\win\prepareimage";$PATH > /dev/null 2>&1
  cmd=""
  prepareimage="prepareimage.exe"
else
  # line for python
  echo "prepareimage with python script"
  prepareimage=$basedir/prepareimage.py
  cmd="python"
fi

echo "$cmd $prepareimage" >> "$projectdir"/output.txt
# Make sure we have a Binary sub-folder in UserApp folder
if [ ! -e $userAppBinary ]; then
mkdir $userAppBinary
fi


command=$cmd" "$prepareimage" enc -k "$oemkey" -i "$iv" "$bin" "$sfu
$command > "$projectdir"/output.txt
ret=$?
if [ $ret -eq 0 ]; then
  command=$cmd" "$prepareimage" sha256 "$bin" "$sign
  $command >> $projectdir"/output.txt"
  ret=$?
  if [ $ret -eq 0 ]; then
    command=$cmd" "$prepareimage" pack -m "$magic" -k "$ecckey" -r 28 -v "$version" -i "$iv" -f "$sfu" -t "$sign" "$sfb" -o "$offset
    $command >> $projectdir"/output.txt"
    ret=$?
    if [ $ret -eq 0 ]; then
      command=$cmd" "$prepareimage" header -m "$magic" -k  "$ecckey" -r 28 -v "$version"  -i "$iv" -f "$sfu" -t "$sign" -o "$offset" "$headerbin
      $command >> $projectdir"/output.txt"
	  command=$cmd" "$prepareimage" header -m "$magic" -k  "$ecckey" -r 28 -v "$version"  -i "$iv" -f "$sfu" -t "$sign" -o "$offset" "$headerbin1
      $command >> $projectdir"/output.txt"
	  command=$cmd" "$prepareimage" header -m "$magic" -k  "$ecckey" -r 28 -v "$version"  -i "$iv" -f "$sfu" -t "$sign" -o "$offset" "$headerbin2
      $command >> $projectdir"/output.txt"
	  command=$cmd" "$prepareimage" header -m "$magic" -k  "$ecckey" -r 28 -v "$version"  -i "$iv" -f "$sfu" -t "$sign" -o "$offset" "$headerbin3
      $command >> $projectdir"/output.txt"
      ret=$?
      if [ $ret -eq 0 ]; then
        command=$cmd" "$prepareimage" merge -v 0 -e 1 -i "$headerbin" -s "$sbsfuelf" -u "$elf" "$bigbinary
        $command >> $projectdir"/output.txt"
        ret=$?
        command=$cmd" "$prepareimage" merge -v 0 -e 1 -i "$headerbin1" -s "$sbsfuelf" -u "$elf" "$bigbinary1
        $command >> $projectdir"/output.txt"
        command=$cmd" "$prepareimage" merge -v 0 -e 1 -i "$headerbin2" -s "$sbsfuelf" -u "$elf" "$bigbinary2
        $command >> $projectdir"/output.txt"
        command=$cmd" "$prepareimage" merge -v 0 -e 1 -i "$headerbin3" -s "$sbsfuelf" -u "$elf" "$bigbinary3
        $command >> $projectdir"/output.txt"

          #Partial image generation if reference userapp exists
          if [ $ret -eq 0 ] && [ -e "$ref_userapp" ]; then
            echo "Generating the partial image .sfb"
            echo "Generating the partial image .sfb" >> $projectdir"/output.txt"
            command=$cmd" "$prepareimage" diff -1 "$ref_userapp" -2 "$bin" "$partialbin" -a "$alignment" --poffset "$partialoffset
            $command >> $projectdir"/output.txt"
            ret=$?
            if [ $ret -eq 0 ]; then
              command=$cmd" "$prepareimage" enc -k "$oemkey" -i "$iv" "$partialbin" "$partialsfu
              $command >> $projectdir"/output.txt"
              ret=$?
              if [ $ret -eq 0 ]; then
                command=$cmd" "$prepareimage" sha256 "$partialbin" "$partialsign
                $command >> $projectdir"/output.txt"
                ret=$?
                if [ $ret -eq 0 ]; then
                  command=$cmd" "$prepareimage" pack -m "$magic" -k "$ecckey" -r 28 -v "$version" -i "$iv" -f "$sfu" -t "$sign" -o "$offset" --pfw "$partialsfu" --ptag "$partialsign" --poffset  "$partialoffset" "$partialsfb
                  $command >> $projectdir"/output.txt"
                  ret=$?
                fi
              fi
            fi
          fi
        if [ $ret -eq 0 ] && [ $# = 6 ]; then
          echo "Generating the global elf file SBSFU and userApp"
          echo "Generating the global elf file SBSFU and userApp" >> $projectdir"/output.txt"
          uname | grep -i -e windows -e mingw > /dev/null 2>&1
          if [ $? -eq 0 ]; then
            # Set to the default installation path of the Cube Programmer tool
            # If you installed it in another location, please update PATH.
            PATH="C:\\Program Files (x86)\\STMicroelectronics\\STM32Cube\\STM32CubeProgrammer\\bin":$PATH > /dev/null 2>&1
            programmertool="STM32_Programmer_CLI.exe"
          else
            which STM32_Programmer_CLI > /dev/null
            if [ $? = 0 ]; then
              programmertool="STM32_Programmer_CLI"
            else
              echo "fix access path to STM32_Programmer_CLI"
            fi
          fi
          command=$programmertool" -ms "$elf" "$headerbin" "$sbsfuelf
          $command >> $projectdir"/output.txt"
          ret=$?
        fi
      fi
    fi
  fi
fi

if [ $ret -eq 0 ]; then
  rm $sign
  rm $sfu
  rm $headerbin
  if [ -e "$ref_userapp" ]; then
    rm $partialbin
    rm $partialsfu
    rm $partialsign
    rm $partialoffset
  fi
  exit 0
else
  echo "$command : failed" >> $projectdir"/output.txt"
  if [ -e  "$elf" ]; then
    rm  $elf
  fi
  if [ -e "$elfbackup" ]; then
    rm  $elfbackup
  fi
  echo $command : failed
  read -n 1 -s
  exit 1
fi
