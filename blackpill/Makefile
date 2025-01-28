CFLAGS  ?=  -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion \
            -g3 -ggdb -Os -ffunction-sections -fdata-sections \
						-I. -Iinclude -I$(CMSISDIR)cmsis_core/CMSIS/Core/Include $(CMSISINCLUDE) \
            -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -std=c++23 -Wno-psabi $(EXTRA_CFLAGS)
LDFLAGS ?= -Tlink.ld -nostartfiles --specs nano.specs -lc -lstdc++ -lgcc -Wl,--gc-sections -Wl,-Map=$@.map 
SOURCES = $(wildcard src/*.c) $(wildcard src/*.cpp)

CMSISDIR = /g/dev/CMSIS/
CMSISINCLUDE = $(addprefix -I, $(wildcard $(CMSISDIR)*/Include))

ifeq ($(OS),Windows_NT)
  RM = rm -f
else
  RM = rm -f
endif

build: clean \
  firmware.bin

build/firmware.elf: $(SOURCES)
	arm-none-eabi-g++ $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@

debug/firmware.debug: build/firmware.elf
	arm-none-eabi-objcopy --only-keep-debug $< $@

firmware.bin: build/firmware.elf debug/firmware.debug
	arm-none-eabi-objcopy -O binary $< $@

flash: firmware.bin
	st-flash --connect-under-reset write $< 0x8000000

cflash: build \
	flash

dflash: firmware.bin
	G:\programs\dfu-util\dfu-util-static.exe -a 0 -s 0x08000000:leave -D $<

cdflash: build \
	dflash

flash_back:
	st-flash --connect-under-reset write stock.bin 0x8000000

dflash_back:
	"C:\Program Files\dfu-util\dfu-util-static.exe" -a 0 -s 0x08000000:leave -D stock.bin

cmsis_core:
	git clone --depth 1 -b v6.1.0 https://github.com/ARM-software/CMSIS_6 $(CMSISDIR)$@

cmsis_f4:
	git clone --depth 1 -b v2.6.10 https://github.com/STMicroelectronics/cmsis_device_f4/ $(CMSISDIR)$@

clean:
	$(RM) build/firmware.* \
	$(RM) debug/firmware.* \
  $(RM) firmware.*