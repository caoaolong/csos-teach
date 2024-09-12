BUILD = ./build
SRC = ./src

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

.PHONY: master
master: $(BUILD)/boot/boot.bin
	dd if=$(BUILD)/boot/boot.bin of=master.img bs=512 count=1 conv=notrunc

.PHONY: bochs
bochs: master
	bochsdbg -q -f bochsrc.bxrc

.PHONY: qemu
qemu: master
	qemu-system-x86_64w -m 128M -drive file=master.img,index=0,media=disk,format=raw

.PHONY: test_chs
test_chs: $(BUILD)/boot/test/read_disk_chs.bin
	dd if=$(BUILD)/boot/test/read_disk_chs.bin of=master.img bs=512 count=1 conv=notrunc
	bochsdbg -q -f bochsrc.bxrc

.PHONY: test_lba
test_lba: $(BUILD)/boot/test/read_disk_lba.bin
	dd if=$(BUILD)/boot/test/read_disk_lba.bin of=master.img bs=512 count=1 conv=notrunc
	bochsdbg -q -f bochsrc.bxrc