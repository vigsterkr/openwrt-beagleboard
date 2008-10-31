#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Image/Prepare
	cp $(LINUX_DIR)/arch/arm/boot/uImage $(KDIR)/uImage
endef

define Image/cmdline/yaffs2
root=/dev/mtdblock4 rootfstype=yaffs2 init=/etc/preinit
endef

define Image/BuildKernel
	$(CP) $(KDIR)/uImage $(BIN_DIR)/openwrt-$(BOARD)-uImage
endef

define Image/Build
	$(CP) $(KDIR)/root.$(1) $(BIN_DIR)/openwrt-$(BOARD)-$(1).fs
	$(CP) $(KDIR)/uImage $(BIN_DIR)/openwrt-$(BOARD)-uImage
endef

$(eval $(call BuildImage))

