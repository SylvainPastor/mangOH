# This makefile depends on targetDefs.* files in the Legato root folder. In order to facilitate the
# inclusion of these files, make needs to be invoked with a --include-dir=$LEGATO_ROOT parameter.
TARGET:=wp85

# Load default PA definitions from main Legato Makefile
$(shell grep 'export PA_DIR .*' $(LEGATO_ROOT)/Makefile > legatoDefs.mk)
$(shell grep 'export .*_PA_.* .*' $(LEGATO_ROOT)/Makefile >> legatoDefs.mk)
include legatoDefs.mk

# Default targets definitions from Framework
include targetDefs.$(TARGET)

.PHONY: all $(TARGET)
all: $(TARGET)

# The comment below is for Developer Studio integration.  Do not remove it.
# DS_CLONE_ROOT(MANGOH_ROOT)
MANGOH_ROOT=$(shell pwd)

# The comment below is for Developer Studio integration.  Do not remove it.
# DS_CUSTOM_OPTIONS(EXTRA_OPTS)
EXTRA_OPTS = -s "$(MANGOH_ROOT)/apps/GpioExpander/gpioExpanderService"

$(TARGET):
	export MANGOH_ROOT=$(MANGOH_ROOT) && \
	mksys \
	$(MKSYS_FLAGS) \
	-t $(TARGET) \
	-i "$(LEGATO_ROOT)/interfaces/supervisor" \
	-i "$(LEGATO_ROOT)/interfaces/positioning" \
	-i "$(LEGATO_ROOT)/interfaces/airVantage" \
	-i "$(LEGATO_ROOT)/interfaces/modemServices" \
	-i "$(LEGATO_ROOT)/interfaces" \
	-i "$(LEGATO_ROOT)/interfaces/atServices" \
	-i "$(LEGATO_ROOT)/interfaces/wifi" \
	-i "$(LEGATO_ROOT)/interfaces/secureStorage" \
	-i "$(LEGATO_ROOT)/interfaces/logDaemon" \
	-s "$(LEGATO_ROOT)/modules/WiFi/service" \
	-s "$(LEGATO_ROOT)/components" \
	-s "$(LEGATO_ROOT)/modules/WiFi/apps/tools/wifi" \
	-s "$(LEGATO_ROOT)/apps/platformServices/airVantage" \
	-s "$(LEGATO_ROOT)/apps/platformServices" \
	-s "$(LEGATO_ROOT)/apps/tools" \
	$(EXTRA_OPTS) \
	-C -g -X -g -L -g \
	mangOH_Green.sdef
	#mangOH_Red.sdef

clean:
	rm -rf /tmp/mkbuild_* _build_* *.update legatoDefs.mk
