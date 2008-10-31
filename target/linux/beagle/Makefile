#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH=arm
BOARD:=beagle
BOARDNAME:=Beagle Board
FEATURES:=squashfs jffs2

LINUX_VERSION:=2.6.27

include $(INCLUDE_DIR)/target.mk

KERNELNAME:="uImage"

$(eval $(call BuildTarget))
$(eval $(call $(if $(CONFIG_TARGET_ROOTFS_ISO),RequireCommand,Ignore),mkisofs, \
   	Please install mkisofs. \
))
