
ARCH = x86_64

CC ?= gcc
LD ?= ld
OBJ ?= objcopy

BASE_INC_DIR = $(shell pkgconf --cflags-only-I gnu-efi)
INCLUDE_DIRS = $(BASE_INC_DIR) $(BASE_INC_DIR)/protocol $(BASE_INC_DIR)/$(ARCH) 
COMPILE_FLAGS = -Wno-error=pragmas -mno-red-zone -mno-avx -fPIE \
		  -g -O2 -Wall -Wextra -Wno-pointer-sign -Werror \
		  -funsigned-char -fshort-wchar -fno-strict-aliasing \
		  -ffreestanding -fno-stack-protector -fno-stack-check -fno-merge-all-constants \
		  -DCONFIG_$(ARCH) -std=c11 -DGNU_EFI_USE_MS_ABI -maccumulate-outgoing-args \
		  -D__KERNEL__ $(WARNINGS)
DEFINES = -D PART_UUID=$(PART_UUID) -D TARGET=$(TARGET)

CFLAGS = $(INCLUDE_DIRS) $(COMPILE_FLAGS) $(DEFINES)

# unfortunately, `pkgconf --libs gnu-efi` only gives `-lefi` while `-lgnuefi` is also required, so I have to list libs manually
LIBS = -lefi -lgnuefi
LIB_DIR = $(shell pkgconf --variable=libdir gnu-efi)
EFI_CRT_OBJS = $(LIB_DIR)/crt0-efi-$(ARCH).o
EFI_LDS = $(LIB_DIR)/elf_$(ARCH)_efi.lds
LDFLAGS = -nostdlib --warn-common --no-undefined --fatal-warnings --no-dynamic-linker \
           --build-id=sha1 -z norelro -z nocombreloc -pie -Bsymbolic -L $(LIB_DIR) 

OBJFLAGS = -j .text -j .sdata -j .data -j .dynamic -j .rodata -j .rel -j .rela \
		   -j .rel.* -j .rela.* -j .rel* -j .rela* -j .areloc -j .reloc \
		   --target=efi-app-$(ARCH)

%.so: %.o
	$(LD) $(LDFLAGS) $(EFI_CRT_OBJS) $^ -o $@ -T $(EFI_LDS) $(LIBS)

%.efi: %.so
	$(OBJ) $(OBJFLAGS) $^ $@

