arch ?= x86_64
build_dir := ../build/arch/$(arch)
driver_files := $(wildcard drivers/*.c) 
driver_headers := $(wildcard drivers/*.h) 
driver_object_files := $(patsubst drivers/%.c, $(build_dir)/drivers/%.o, $(driver_files))

lib_files := $(wildcard lib/*.c) 
lib_object_files := $(patsubst lib/%.c, $(build_dir)/libs/%.o, $(lib_files))
lib_headers := $(wildcard *.h) 

CC = ../bin/$(arch)-elf-gcc 
CFLAGS = -Wall -g

.PHONY: all drivers libs

all: drivers libs

drivers: $(driver_object_files)

$(build_dir)/drivers/%.o: drivers/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link library object files into one object file.
libs: $(lib_object_files)
	#mkdir -p $(build_dir)/libs
	#$(CC) $(CFLAGS) -o $(build_dir)/libs/libc-$(arch).o $(lib_object_files)

$(build_dir)/libs/%.o: lib/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@
