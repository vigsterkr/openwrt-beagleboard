#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Ralink
  NAME:=Ralink WiFi
  PACKAGES:=kmod-rt2500
endef

define Profile/Ralink/Description
        Package set compatible with hardware using Ralink WiFi cards
endef
$(eval $(call Profile,Ralink))