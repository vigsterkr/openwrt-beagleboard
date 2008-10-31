#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libdspbridge
PKG_REV:=0657be4d462aa8bfbf2e5d5d18623d7a69f0a113
PKG_VERSION:=20081012
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=git://github.com/felipec/libdspbridge.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)

include $(INCLUDE_DIR)/package.mk

define Package/libdspbridge
  SECTION:=libs
  DEPENDS:=+kmod-dsp-bridge
  CATEGORY:=Libraries
  TITLE:=Library for OMAP DSP
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS)
#		CFLAGS="$(TARGET_CFLAGS)"
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		install
endef

define Build/InstallDev
	mkdir -p $(1)/usr/lib
	$(CP)   $(PKG_INSTALL_DIR)/usr/lib/libbridge.so* \
		$(1)/usr/lib/
endef

define Package/libdspbridge/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbridge.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdspbridge))

