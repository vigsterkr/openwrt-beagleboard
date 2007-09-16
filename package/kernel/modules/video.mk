#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

VIDEO_MENU:=Video Support

define KernelPackage/video-core
  SUBMENU:=$(VIDEO_MENU)
  TITLE=Video4Linux support
  KCONFIG:= \
	CONFIG_VIDEO_DEV \
	CONFIG_VIDEO_V4L1=y \
	CONFIG_VIDEO_CAPTURE_DRIVERS=y \
	CONFIG_V4L_USB_DRIVERS=y 
endef

define KernelPackage/video-core/2.4
  FILES:=$(LINUX_DIR)/drivers/media/video/videodev.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,60,videodev)
endef

define KernelPackage/video-core/2.6
  FILES:= \
	$(LINUX_DIR)/drivers/media/video/v4l2-common.$(LINUX_KMOD_SUFFIX) \
	$(LINUX_DIR)/drivers/media/video/v4l1-compat.$(LINUX_KMOD_SUFFIX) \
	$(LINUX_DIR)/drivers/media/video/compat_ioctl32.$(LINUX_KMOD_SUFFIX) \
	$(LINUX_DIR)/drivers/media/video/videodev.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,60, \
	v4l2-common \
	v4l1-compat \
	compat_ioctl32 \
	videodev \
  )
endef

define KernelPackage/video-core/description
 Kernel modules for Video4Linux support
endef

$(eval $(call KernelPackage,video-core))


define KernelPackage/video-pwc
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Philips webcam support
  DEPENDS:=@LINUX_2_6 @USB_SUPPORT +kmod-usb-core +kmod-video-core
  KCONFIG:= \
	CONFIG_USB_PWC \
	CONFIG_USB_PWC_DEBUG=n
  FILES:=$(LINUX_DIR)/drivers/media/video/pwc/pwc.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,70,pwc)
endef


define KernelPackage/video-pwc/description
 Kernel modules for supporting Philips USB based cameras.
endef

$(eval $(call KernelPackage,video-pwc))


define KernelPackage/video-cpia2
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=CPIA2 video driver
  DEPENDS:=@LINUX_2_6 @USB_SUPPORT +kmod-usb-core +kmod-video-core
  KCONFIG:=CONFIG_VIDEO_CPIA2
  FILES:=$(LINUX_DIR)/drivers/media/video/cpia2/cpia2.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,70,cpia2)
endef

define KernelPackage/video-cpia2/description
 Kernel modules for supporting CPIA2 USB based cameras.
endef

$(eval $(call KernelPackage,video-cpia2))