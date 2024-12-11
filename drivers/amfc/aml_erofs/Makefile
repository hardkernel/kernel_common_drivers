# SPDX-License-Identifier: GPL-2.0-only

MODULE_NAME = amlogic-soc-erofs

$(MODULE_NAME)-$(CONFIG_AMLOGIC_EROFS) := super.o inode.o data.o namei.o dir.o sysfs.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_EROFS_XATTR) += xattr.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_EROFS_ZIP) += decompressor.o zmap.o zdata.o zutil.o decompressor_cryptor.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_EROFS_ZIP_LZMA) += decompressor_lzma.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_EROFS_ZIP_DEFLATE) += decompressor_deflate.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_EROFS_ZIP_ZSTD) += decompressor_zstd.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_EROFS_BACKED_BY_FILE) += fileio.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_EROFS_ONDEMAND) += fscache.o

obj-$(CONFIG_AMLOGIC_EROFS) = $(MODULE_NAME).o
