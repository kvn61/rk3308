0S2MONO_VERSION = 1.0
0S2MONO_SITE = /home/slava/my/br/3308/src/s2mono
0S2MONO_SITE_METHOD=local
0S2MONO_DEPENDENCIES = alsa-lib

define 0S2MONO_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/lib/alsa-lib
	$(INSTALL) -D -m 755  $(@D)/*.so $(TARGET_DIR)/usr/lib/alsa-lib
endef

define 0S2MONO_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

$(eval $(generic-package))
