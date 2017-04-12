arch ?= x86_64
build_dir := build/arch/$(arch)
kernel := build/kernel.bin
iso := build/os-$(arch).iso
img := build/dankos.img

linker_script := src/asm/linker.ld
grub_cfg := config/grub.cfg
assembly_source_files := $(wildcard src/asm/*.asm)
assembly_object_files := $(patsubst src/asm/%.asm, \
	build/arch/$(arch)/%.o, $(assembly_source_files))

c_source := $(wildcard src/*.c)
c_object_files := $(patsubst src/%.c, \
	build/arch/$(arch)/%.o, $(c_source))
driver_files := src/drivers/*.c
drv_object_files := $(patsubst src/drivers/%.c, $(build_dir)/drivers/%.o, \
	$(driver_files))
#libc := $(build_dir)/libs/libc-$(arch).o
lib_files := $(wildcard src/lib/*.c)
lib_obj_files := $(patsubst src/lib/%.c, $(build_dir)/libs/%.o, $(lib_files))

CC = bin/$(arch)-elf-gcc
CFLAGS = -Wall -g -c

.PHONY: all clean run img

all: update-img

clean:
	rm -r build

run: img
	qemu-system-x86_64 -s -drive format=raw,file=$(img) -serial stdio

img: update-img

make-img:
	@if [ ! -s $(img) ]; then \
		echo "Image not found. Creating it..."; \
		dd if=/dev/zero of=$(img) bs=512 count=1048576 ; \
		sudo parted $(img) mklabel msdos ; \
		sudo parted $(img) mkpart primary ext2 2048s 1046528s ; \
		sudo parted $(img) set 1 boot on ; \
		sudo losetup /dev/loop0 $(img) ; \
		sudo losetup /dev/loop1 $(img) -o 1048576 ; \
		sudo mkfs.ext2 /dev/loop1 ; \
		sudo mount /dev/loop1 /mnt/loop ; \
		sudo grub-install --root-directory=/mnt/loop --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop0 ; \
		sudo losetup -d /dev/loop0 ; \
		sudo losetup -d /dev/loop1 ; \
		sudo umount /mnt/loop ; \
	fi;

update-img: $(kernel) $(grup_cfg) make-img
	@echo "Updating image file..."
	@sudo losetup /dev/loop0 $(img)
	@sudo losetup /dev/loop1 $(img) -o 1048576
	@sudo mount /dev/loop1 /mnt/loop
	@sudo mkdir -p /mnt/loop/boot/grub
	@sudo cp $(grub_cfg) /mnt/loop/boot/grub
	@sudo cp $(kernel) /mnt/loop/boot
	@sudo umount /mnt/loop
	@sudo losetup -d /dev/loop0
	@sudo losetup -d /dev/loop1

$(kernel): $(assembly_object_files) $(linker_script) $(c_object_files) $(drv_object_files)
	cd src && $(MAKE) && cd ..
	ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(c_object_files) $(lib_obj_files) $(drv_object_files)

# compile c files
build/arch/$(arch)/%.o: src/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) $< -o $@
	
# compile assembly files
build/arch/$(arch)/%.o: src/asm/%.asm
	mkdir -p $(shell dirname $@)
	nasm -felf64 $< -o $@
