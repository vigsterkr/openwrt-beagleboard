# 
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

#define Image/Prepare
#	cp $(LINUX_DIR)/arch/arm/boot/zImage $(KDIR)/zImage
#endef

define Image/BuildKernel
	cp $(KDIR)/vmlinux $(BIN_DIR)/openwrt-$(BOARD)-vmlinux
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

define Image/Build/jffs2-64k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/openwrt-$(BOARD)-$(1).img bs=65536 conv=sync
endef

define Image/Build/jffs2-128k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/openwrt-$(BOARD)-$(1).img bs=131072 conv=sync
	$(call Image/Build/slug,$(1))
endef

define Image/Build/squashfs
    $(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/openwrt-$(BOARD)-$(1).img bs=131072 conv=sync
	$(call Image/Build/slug,$(1))
endef

$(eval $(call BuildImage))
