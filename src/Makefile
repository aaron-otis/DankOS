arch ?= x86_64
build_dir := ../build/arch/$(arch)
int_script := generate_interrupt_code.py

driver_files := $(wildcard drivers/*.c) 
driver_headers := $(wildcard drivers/*.h) 
driver_object_files := $(patsubst drivers/%.c, $(build_dir)/drivers/%.o, $(driver_files))

lib_files := $(wildcard lib/*.c) 
lib_object_files := $(patsubst lib/%.c, $(build_dir)/libs/%.o, $(lib_files))
lib_headers := $(wildcard *.h) 

sys_files := $(wildcard sys/*.c)
sys_obj_files := $(patsubst sys/%.c, $(build_dir)/sys/%.o, $(sys_files))

CC = ../bin/$(arch)-elf-gcc 
CFLAGS = -Wall -g

.PHONY: all drivers libs

all: drivers libs sys

drivers: gen_code $(driver_object_files)

$(build_dir)/drivers/%.o: drivers/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

libs: $(lib_object_files)

$(build_dir)/libs/%.o: lib/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

gen_code: $(int_script)
	@echo "Generating interrupt code."
	@python $(int_script)

sys: $(sys_obj_files)

$(build_dir)/sys/%.o: sys/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@
