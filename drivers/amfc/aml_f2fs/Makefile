# SPDX-License-Identifier: GPL-2.0

MODULE_NAME = amlogic-soc-f2fs

ifdef CONFIG_CFI_CLANG
KBUILD_CFLAGS := $(filter-out -fno-sanitize-blacklist, $(KBUILD_CFLAGS))
KBUILD_CFLAGS += -fsanitize-ignorelist=$(srctree)/common_drivers/drivers/amfc/aml_f2fs/cfi_ignore.txt
endif

$(MODULE_NAME)-y		:= dir.o file.o inode.o namei.o hash.o super.o inline.o
$(MODULE_NAME)-y		+= checkpoint.o gc.o data.o node.o segment.o recovery.o
$(MODULE_NAME)-y		+= shrinker.o extent_cache.o sysfs.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_F2FS_STAT_FS) += debug.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_F2FS_FS_XATTR) += xattr.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_F2FS_FS_POSIX_ACL) += acl.o
$(MODULE_NAME)-$(CONFIG_FS_VERITY) += verity.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_F2FS_FS_COMPRESSION) += compress.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_F2FS_IOSTAT) += iostat.o

obj-$(CONFIG_AMLOGIC_F2FS_FS) = $(MODULE_NAME).o
