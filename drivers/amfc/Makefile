# SPDX-License-Identifier: GPL-2.0-only
# Amlogic amfc drivers

MODULE_NAME1 = amlogic-amfc
$(MODULE_NAME1)-$(CONFIG_AMLOGIC_AMFC) = amfc.o
obj-$(CONFIG_AMLOGIC_AMFC) = $(MODULE_NAME1).o

#hostprogs += zstd

ifeq ($(srctree),)
srctree := $(patsubst %/,%,$(dir $(CURDIR)))
srctree := $(patsubst %/,%,$(dir $(srctree)))
endif

zstd_src := $(srctree)/common_drivers/drivers/amfc/zstd_tool/common/debug.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/common/entropy_common.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/common/error_private.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/common/fse_decompress.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/common/pool.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/common/threading.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/common/xxhash.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/common/zstd_common.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/fse_compress.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/hist.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/huf_compress.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_compress.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_compress_literals.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_compress_sequences.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_compress_superblock.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_double_fast.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_fast.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_lazy.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_ldm.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstd_opt.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/compress/zstdmt_compress.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/decompress/huf_decompress.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/decompress/huf_decompress_amd64.S
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/decompress/zstd_ddict.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/decompress/zstd_decompress.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/decompress/zstd_decompress_block.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/dictBuilder/cover.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/dictBuilder/divsufsort.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/dictBuilder/fastcover.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/dictBuilder/zdict.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/legacy/zstd_v05.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/legacy/zstd_v06.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/legacy/zstd_v07.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/benchfn.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/benchzstd.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/datagen.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/dibio.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/fileio.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/timefn.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/util.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/zstdcli.c
zstd_src += $(srctree)/common_drivers/drivers/amfc/zstd_tool/zstdcli_trace.c

$(obj)/zstd:
	$(HOSTCC) $(zstd_src) $(LDFLAGS) -o $@
	cp $@ $(srctree)/../build/kernel/build-tools/path/linux-x86/

$(obj)/amfc.o: ${obj}/zstd

obj-$(CONFIG_AMLOGIC_EROFS) += aml_erofs/
