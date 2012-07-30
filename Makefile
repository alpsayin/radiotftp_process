SRC=fibonacci
RADIOTFTP_SOURCEFILES=ax25.c ethernet.c manchester.c tftp.c timers.c udp_ip.c util.c printAsciiHex.c radiotftp_process.c

PROJECT_SOURCEFILES+=$(RADIOTFTP_SOURCEFILES)

TARGET=avr-atmega128rfa1
CONTIKI=/home/alpsayin/contiki

all: cleanExec $(SRC) $(SRC).lss $(SRC).hex $(SRC).eep $(SRC).size

include $(CONTIKI)/Makefile.include

cleanExec:
	rm -rf *.hex
	rm -rf *.eep

$(SRC).lss: $(SRC).$(TARGET)
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S $(SRC).$(TARGET)  >"$(SRC).lss"
	@echo 'Finished building: $@'
	@echo ' '

$(SRC).hex: $(SRC).$(TARGET)
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .signature -O ihex $(SRC).$(TARGET)  "$(SRC).hex"
	@echo 'Finished building: $@'
	@echo ' '

$(SRC).eep: $(SRC).$(TARGET)
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --no-change-warnings --change-section-lma .eeprom=0 -O ihex $(SRC).$(TARGET)  "$(SRC).eep"
	@echo 'Finished building: $@'
	@echo ' '

$(SRC).size: $(SRC).$(TARGET)
	@echo 'Invoking: Print Size'
	-avr-size --format=berkeley -t $(SRC).$(TARGET)
	@echo 'Finished building: $@'
	@echo ' '
	
install: all
	-avrdude -p m128rfa1 -c avrispmkII -P usb -U eeprom:w:$(SRC).eep
	-avrdude -p m128rfa1 -c avrispmkII -P usb -U flash:w:$(SRC).hex