

# Copyright (c) 2013 Andy Little
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>


############################################################
# **** you will need modify the paths in this section to the paths you saved the libraries in***


# the STM32F4 librraies are available from
# http://www.st.com/st-web-ui/static/active/en/st_prod_software_internet/resource/technical/software/firmware/stm32f4_dsp_stdperiph_lib.zip
# Change this to the path where you saved the STM32F4 standard Peripheral libraries
#STM32F4_INCLUDE_PATH = /opt/stm32f4/STM32F4xx_DSP_StdPeriph_Lib_V1.0.0/Libraries/
STM32F4_INCLUDE_PATH = /home/andy/STM32Cube/Repository/STM32Cube_FW_F4_V1.13.0/Drivers/
# The quan libraries are available from
# https://github.com/kwikius/quan-trunk
# Change this to the the path twhere you saved the quan libraries
#QUAN_INCLUDE_PATH = /home/andy/website/quan-trunk/
QUAN_INCLUDE_PATH = /home/andy/cpp/projects/quan-trunk/
# The GCC ARM embedded toolchain (recommended) is available from
#  https://launchpad.net/gcc-arm-embedded
# If using this toolchain, the TOOLCHAIN_ID should be set to GCC_Arm_Embedded (The default)
#TOOLCHAIN_ID = GCC_Arm_Embedded
# Otherwise if you are using the toolchain from
# https://github.com/prattmic/arm-cortex-m4-hardfloat-toolchain
# set the TOOLCHAIN_ID as follows
#TOOLCHAIN_ID = Michael_Pratt

# Change this to the path where you installed the  arm gcc compiler toolchain
#TOOLCHAIN_PREFIX = /opt/gcc-arm-none-eabi-4_7-2013q2/
#TOOLCHAIN_PREFIX = /home/andy/andy_bin/gcc-arm-none-eabi-5_2-2015q4/
TOOLCHAIN_PREFIX = /home/andy/andy_bin/gcc-arm-none-eabi-4_9-2015q3/
# Change this to your version of gcc.
# (You can find the gcc version by invoking arm-noe-eabi-gcc --version in the $(TOOLCHAIN_PREFIX)/bin/ directory)
#TOOLCHAIN_GCC_VERSION = 4.7.4
#TOOLCHAIN_GCC_VERSION = 5.2.1
TOOLCHAIN_GCC_VERSION = 4.9.3
INIT_LIB_PREFIX = $(TOOLCHAIN_PREFIX)/lib/gcc/arm-none-eabi/$(TOOLCHAIN_GCC_VERSION)/armv7e-m/fpu/

CC      = $(TOOLCHAIN_PREFIX)bin/arm-none-eabi-g++
CC1     = $(TOOLCHAIN_PREFIX)bin/arm-none-eabi-gcc
LD      = $(TOOLCHAIN_PREFIX)bin/arm-none-eabi-g++
CP      = $(TOOLCHAIN_PREFIX)bin/arm-none-eabi-objcopy
OD      = $(TOOLCHAIN_PREFIX)bin/arm-none-eabi-objdump
SIZ     = $(TOOLCHAIN_PREFIX)bin/arm-none-eabi-size

STM32F4_LINKER_SCRIPT = system/stm32f4.ld

INCLUDES = -I$(STM32F4_INCLUDE_PATH)CMSIS/Include/ \
-I$(STM32F4_INCLUDE_PATH)CMSIS/Device/ST/STM32F4xx/Include/ \
-I$(STM32F4_INCLUDE_PATH)STM32F4xx_StdPeriph_Driver/inc/ \
-I$(QUAN_INCLUDE_PATH)

CFLAG_EXTRAS = -fno-math-errno -DQUAN_STM32F4 -DQUAN_NO_EXCEPTIONS -DSTM32F405xx -DSTM32F40_41xxx -DQUAN_SYSTICK_TIMER_UINT32 -DHSE_VALUE=8000000
CFLAG_EXTRAS += -Wl,-u,vsprintf -lm
CFLAG_EXTRAS += -DDEBUG
CFLAG_EXTRAS += -Wall -Wno-unused-local-typedefs
CFLAG_EXTRAS += -fmax-errors=1

PROCESSOR_FLAGS = -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mthumb -mfloat-abi=hard

CFLAGS = -std=c++11 -fno-rtti -fno-exceptions -c -Os -g  $(INCLUDES) $(PROCESSOR_FLAGS) $(CFLAG_EXTRAS) \
-fdata-sections -ffunction-sections

INIT_LIBS = $(INIT_LIB_PREFIX)crti.o $(INIT_LIB_PREFIX)crtn.o

LFLAGS = -T$(STM32F4_LINKER_SCRIPT) -Os $(PROCESSOR_FLAGS) $(CFLAG_EXTRAS) $(INIT_LIBS) -nodefaultlibs -nostartfiles \
--specs=nano.specs -Wl,--gc-sections

CPFLAGS = -Obinary
ODFLAGS = -d

STARTUP = startup.s

OBJDIR = ./obj/

BINDIR = ./bin/

eeprom_objects = $(patsubst %,$(OBJDIR)%,eeprom_writer.o eeprom_reader.o eeprom_test.o)

compass_objects = $(patsubst %,$(OBJDIR)%,compass.o compass_test.o)

system_objects = $(patsubst %,$(OBJDIR)%,serial_port.o i2c.o led.o setup.o spbrk.o system_init.o)

local_objects = $(patsubst %,$(OBJDIR)%,main.o)

lis3_mdl_objects = $(patsubst %,$(OBJDIR)%,lis3_mdl.o lis3_mdl_test.o)

i2c_driver_objects = $(patsubst %,$(OBJDIR)%,i2c_driver.o)

objects  = $(local_objects) $(eeprom_objects) $(compass_objects) $(system_objects) \
	$(lis3_mdl_objects) $(i2c_driver_objects) $(OBJDIR)startup.o

all: test

Debug : test

clean:
	-rm -rf $(OBJDIR)*.o $(BINDIR)*.elf $(BINDIR)*.bin $(BINDIR)*.lst

test: $(BINDIR)main.elf
	@ echo "...copying"
	$(CP) $(CPFLAGS)  $(BINDIR)main.elf  $(BINDIR)main.bin
	$(OD) $(ODFLAGS)  $(BINDIR)main.elf >  $(BINDIR)main.lst
	$(SIZ) -A  $(BINDIR)main.elf

$(BINDIR)main.elf : $(objects)
	@ echo "..linking"
	$(LD)   $(LFLAGS) -o $(BINDIR)main.elf $(objects)

$(local_objects): $(OBJDIR)%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

$(system_objects): $(OBJDIR)%.o : system/%.cpp
	$(CC) $(CFLAGS) $< -o $@

$(eeprom_objects): $(OBJDIR)%.o : eeprom/%.cpp
	$(CC) $(CFLAGS) $< -o $@

$(compass_objects): $(OBJDIR)%.o : compass/%.cpp
	$(CC) $(CFLAGS) $< -o $@

$(lis3_mdl_objects): $(OBJDIR)%.o : lis3_mdl/%.cpp
	$(CC) $(CFLAGS) $< -o $@

$(i2c_driver_objects): $(OBJDIR)%.o : i2c_driver/%.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)startup.o: system/$(STARTUP)
	$(CC) $(CFLAGS) $< -o $@

#upload : test
#	st-flash write $(BINDIR)main.bin 0x8000000

upload : test
	/home/andy/bin/stm32flash -b 115200 -f -v -w $(BINDIR)main.bin /dev/ttyUSB0

