# 
# Copyright (C) 2006 OpenWrt.org
# Copyright (C) 2008 Luigi 'Comio' Mantellini <luigi.mantellini@idf-hit.com>
#                    Industrie Dial Face S.p.A. 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gmp
PKG_VERSION:=4.2.2

PKG_SOURCE_URL:=http://ftp.sunet.se/pub/gnu/gmp/ \
		ftp://ftp.gnu.org/gnu/gmp/

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PATCH_DIR:=./patches/$(PKG_VERSION)
# STAGING_DIR_HOST:=$(TOOLCHAIN_DIR)
BUILD_DIR_HOST:=$(BUILD_DIR_TOOLCHAIN)

include $(INCLUDE_DIR)/host-build.mk

define Build/Configure
	(cd $(BUILD_DIR_HOST); \
	 ln -sf $(PKG_NAME)-$(PKG_VERSION) $(PKG_NAME) \
	);
	(cd $(PKG_BUILD_DIR); \
		./configure \
		--enable-fft \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_HOST_NAME) \
		--prefix=$(STAGING_DIR_HOST) \
	);
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
#	$(MAKE) -C $(PKG_BUILD_DIR) check
endef

define Build/Install
	$(MAKE) -C $(PKG_BUILD_DIR) install
endef

$(eval $(call HostBuild))