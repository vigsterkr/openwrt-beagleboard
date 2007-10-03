#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifneq ($(DUMP),)
  all: dumpinfo
endif

ifneq ($(__target_inc),1)
__target_inc=1

TARGET_BUILD?=0


target_conf=$(subst .,_,$(subst -,_,$(subst /,_,$(1))))
ifeq ($(DUMP),)
  PLATFORM_DIR:=$(TOPDIR)/target/linux/$(BOARD)
  SUBTARGET:=$(strip $(foreach subdir,$(patsubst $(PLATFORM_DIR)/%/target.mk,%,$(wildcard $(PLATFORM_DIR)/*/target.mk)),$(if $(CONFIG_TARGET_$(call target_conf,$(BOARD)_$(subdir))),$(subdir))))
else
  PLATFORM_DIR:=${CURDIR}
endif

TARGETID:=$(BOARD)$(if $(SUBTARGET),/$(SUBTARGET))
PLATFORM_SUBDIR:=$(PLATFORM_DIR)$(if $(SUBTARGET),/$(SUBTARGET))

define Target
  KERNEL_TARGETS+=$(1)
  ifeq ($(DUMP),1)
    ifeq ($(SUBTARGET),)
      BuildTarget=$$(BuildTargets/DumpAll)
    endif
  endif
endef

ifneq ($(TARGET_BUILD),1)
  include $(PLATFORM_DIR)/Makefile
  ifneq ($(PLATFORM_DIR),$(PLATFORM_SUBDIR))
    include $(PLATFORM_SUBDIR)/target.mk
  endif
else
  ifneq ($(SUBTARGET),)
    -include ./$(SUBTARGET)/target.mk
  endif
endif

define Profile/Default
  NAME:=
  PACKAGES:=
endef

define Profile
  $(eval $(call Profile/Default))
  $(eval $(call Profile/$(1)))
  $(eval $(call shexport,Profile/$(1)/Config))
  $(eval $(call shexport,Profile/$(1)/Description))
  DUMPINFO += \
	echo "Target-Profile: $(1)"; \
	echo "Target-Profile-Name: $(NAME)"; \
	echo "Target-Profile-Packages: $(PACKAGES)"; \
	if [ -f ./config/profile-$(1) ]; then \
		echo "Target-Profile-Kconfig: yes"; \
	fi; \
	echo "Target-Profile-Config: "; \
	getvar "$(call shvar,Profile/$(1)/Config)"; \
	echo "@@"; \
	echo "Target-Profile-Description:"; \
	getvar "$(call shvar,Profile/$(1)/Description)"; \
	echo "@@"; \
	echo;
  ifeq ($(CONFIG_TARGET_$(call target_conf,$(BOARD)_$(if $(SUBTARGET),$(SUBTARGET)_)$(1))),y)
    PROFILE=$(1)
  endif
endef

ifneq ($(PLATFORM_DIR),$(PLATFORM_SUBDIR))
  define IncludeProfiles
    -include $(PLATFORM_DIR)/profiles/*.mk
    -include $(PLATFORM_SUBDIR)/profiles/*.mk
  endef
else
  define IncludeProfiles
    -include $(PLATFORM_DIR)/profiles/*.mk
  endef
endif

ifeq ($(TARGET_BUILD),1)
  $(eval $(call IncludeProfiles))
else
  ifeq ($(DUMP),)
    $(eval $(call IncludeProfiles))
  endif
endif

$(eval $(call shexport,Target/Description))

include $(INCLUDE_DIR)/kernel-version.mk

GENERIC_PLATFORM_DIR := $(TOPDIR)/target/linux/generic-$(KERNEL)
GENERIC_PATCH_DIR := $(GENERIC_PLATFORM_DIR)/patches$(shell [ -d "$(GENERIC_PLATFORM_DIR)/patches-$(KERNEL_PATCHVER)" ] && printf -- "-$(KERNEL_PATCHVER)" || true )

GENERIC_LINUX_CONFIG?=$(firstword $(wildcard $(GENERIC_PLATFORM_DIR)/config-$(KERNEL_PATCHVER) $(GENERIC_PLATFORM_DIR)/config-default))
LINUX_CONFIG?=$(firstword $(wildcard $(foreach subdir,$(PLATFORM_DIR) $(PLATFORM_SUBDIR),$(subdir)/config-$(KERNEL_PATCHVER) $(subdir)/config-default)))
LINUX_SUBCONFIG?=$(firstword $(wildcard $(PLATFORM_SUBDIR)/config-$(KERNEL_PATCHVER) $(PLATFORM_SUBDIR)/config-default))
ifeq ($(LINUX_CONFIG),$(LINUX_SUBCONFIG))
  LINUX_SUBCONFIG:=
endif
LINUX_CONFCMD=$(if $(LINUX_CONFIG),$(SCRIPT_DIR)/kconfig.pl + $(GENERIC_LINUX_CONFIG) $(if $(LINUX_SUBCONFIG),+ $(LINUX_CONFIG) $(LINUX_SUBCONFIG),$(LINUX_CONFIG)),true)

ifeq ($(DUMP),1)
  BuildTarget=$(BuildTargets/DumpCurrent)

  ifneq ($(BOARD),)
    TMP_CONFIG:=$(TMP_DIR)/.kconfig-$(call target_conf,$(TARGETID))
    $(TMP_CONFIG): $(GENERIC_LINUX_CONFIG) $(LINUX_CONFIG) $(LINUX_SUBCONFIG)
		$(LINUX_CONFCMD) > $@ || rm -f $@
    -include $(TMP_CONFIG)
    .SILENT: $(TMP_CONFIG)
    .PRECIOUS: $(TMP_CONFIG)

    ifneq ($(CONFIG_PCI),)
      FEATURES += pci
    endif
    ifneq ($(CONFIG_USB),)
      FEATURES += usb
    endif
    ifneq ($(CONFIG_PCMCIA)$(CONFIG_PCCARD),)
      FEATURES += pcmcia
    endif

    # remove duplicates
    FEATURES:=$(sort $(FEATURES))
  endif
endif

define BuildTargets/DumpAll
  dumpinfo:
	@$(foreach SUBTARGET,$(KERNEL_TARGETS),$(SUBMAKE) -s DUMP=1 SUBTARGET=$(SUBTARGET); )
endef

define BuildTargets/DumpCurrent

  dumpinfo:
	@echo 'Target: $(TARGETID)'; \
	 echo 'Target-Board: $(BOARD)'; \
	 echo 'Target-Kernel: $(KERNEL)'; \
	 echo 'Target-Name: $(BOARDNAME) [$(KERNEL)]'; \
	 echo 'Target-Path: $(subst $(TOPDIR)/,,$(PWD))'; \
	 echo 'Target-Arch: $(ARCH)'; \
	 echo 'Target-Features: $(FEATURES)'; \
	 echo 'Linux-Version: $(LINUX_VERSION)'; \
	 echo 'Linux-Release: $(LINUX_RELEASE)'; \
	 echo 'Linux-Kernel-Arch: $(LINUX_KARCH)'; \
	 echo 'Target-Description:'; \
	 getvar $(call shvar,Target/Description); \
	 echo '@@'; \
	 echo 'Default-Packages: $(DEFAULT_PACKAGES)'; \
	 $(DUMPINFO)
endef

include $(INCLUDE_DIR)/kernel.mk
ifeq ($(TARGET_BUILD),1)
  include $(INCLUDE_DIR)/kernel-build.mk
  BuildTarget?=$(BuildKernel)
endif

endif #__target_inc
