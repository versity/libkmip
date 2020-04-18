## 
## Build libkmip for Versity development
## 


# --------------------------------------------------------------------------
# Set libkmip version number using git tags and optional developer PKG_PATCH
# environment variable

GIT-VERSION-FILE: .FORCE
	./GIT-VERSION-GEN
-include GIT-VERSION-FILE

LIBKMIP_VER_MAJOR ?= $(wordlist 1,1, $(subst -, ,$(subst ., ,$(VERSION))))
LIBKMIP_VER_MINOR ?= $(wordlist 2,2, $(subst -, ,$(subst ., ,$(VERSION))))
LIBKMIP_VER := $(LIBKMIP_VER_MAJOR).$(LIBKMIP_VER_MINOR)


# --------------------------------------------------------------------------
# BUILD directory

ifndef BUILD
    ifdef DEBUG
        BUILD := build-debug
    else
        BUILD := build
    endif
endif

$(BUILD):
	mkdir -p $(BUILD)

# --------------------------------------------------------------------------
# DESTDIR directory

ifndef DESTDIR
    DESTDIR := /usr
endif

# --------------------------------------------------------------------------
# LIBDIR directory

ifndef LIBDIR
    LIBDIR := $(DESTDIR)/lib
endif

# --------------------------------------------------------------------------
# Library dependencies for libkmip

ifndef OPENSSL_LIBS
    OPENSSL_LIBS := -lssl -lcrypto
endif


# --------------------------------------------------------------------------
# These CFLAGS assume a GNU compiler.  For other compilers, write a script
# which converts these arguments into their equivalent for that particular
# compiler.

CFLAGS = -std=c11 \
         -pedantic \
         -Wall \
         -Wextra \
         -D__STRICT_ANSI__ \
         -D_ISOC99_SOURCE \
         -D_POSIX_C_SOURCE=200112L

ifdef DEBUG
    CFLAGS += -Og -g3
else
    CFLAGS += -O3
endif

LDFLAGS = $(OPENSSL_LIBS)

AR      = ar
ARFLAGS = csrv


# --------------------------------------------------------------------------
# Default targets are everything

.PHONY: all
all: exported tests demos


# --------------------------------------------------------------------------
# Exported targets are the library and utility programs

.PHONY: exported
exported: libkmip kmip-get headers


# --------------------------------------------------------------------------
# Install target

.PHONY: install
install: exported
	@echo Installing executable: $(DESTDIR)/bin/kmip-get
	install -Dps -m u+rwx,go+rx \
	    $(BUILD)/bin/kmip-get \
	    $(DESTDIR)/bin/kmip-get
	@echo Installing shared library: $(LIBDIR)/libkmip.so.$(LIBKMIP_VER)
	install -Dps -m u+rw,go+r \
	    $(BUILD)/lib/libkmip.so.$(LIBKMIP_VER_MAJOR) \
	    $(LIBDIR)/libkmip.so.$(LIBKMIP_VER)
	@echo Linking shared library: $(LIBDIR)/libkmip.so.$(LIBKMIP_VER_MAJOR)
	ln -sf libkmip.so.$(LIBKMIP_VER) \
	    $(LIBDIR)/libkmip.so.$(LIBKMIP_VER_MAJOR)
	@echo Linking shared library: $(LIBDIR)/libkmip.so
	ln -sf libkmip.so.$(LIBKMIP_VER_MAJOR) $(LIBDIR)/libkmip.so
	@echo Installing static library: $(LIBDIR)/libkmip.a
	install -Dp -m u+rw,go+r $(BUILD)/lib/libkmip.a \
	    $(LIBDIR)/libkmip.a
	@echo Installing header: $(DESTDIR)/include/kmip.h
	install -Dp -m u+rw,go+r $(BUILD)/include/kmip.h \
	    $(DESTDIR)/include/kmip.h
	@echo Installing header: $(DESTDIR)/include/kmip_bio.h
	install -Dp -m u+rw,go+r $(BUILD)/include/kmip_bio.h \
	    $(DESTDIR)/include/kmip_bio.h
	@echo Installing header: $(DESTDIR)/include/kmip_memset.h
	install -Dp -m u+rw,go+r $(BUILD)/include/kmip_memset.h \
	    $(DESTDIR)/include/kmip_memset.h


# --------------------------------------------------------------------------
# Uninstall target

.PHONY: uninstall
uninstall:
	@echo Uninstalled files: ...
	rm -f \
	    $(DESTDIR)/bin/kmip-get \
	    $(DESTDIR)/include/kmip.h \
	    $(DESTDIR)/include/kmip_bio.h \
	    $(DESTDIR)/include/kmip_memset.h \
	    $(DESTDIR)/lib/libkmip.a \
	    $(DESTDIR)/lib/libkmip.so \
	    $(DESTDIR)/lib/libkmip.so.$(LIBKMIP_VER_MAJOR) \
	    $(DESTDIR)/lib/libkmip.so.$(LIBKMIP_VER)


# --------------------------------------------------------------------------
# Compile target patterns

$(BUILD)/obj/%.o: %.c
	@echo Compiling object: $@
	@mkdir -p $(dir $(BUILD)/dep/$<)
	@$(CC) $(CFLAGS) -M -MG -MQ $@ -o $(BUILD)/dep/$(<:%.c=%.d) -c $<
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/obj/%.lo: %.c
	@echo Compiling dynamic object: $@
	@mkdir -p $(dir $(BUILD)/dep/$<)
	@$(CC) $(CFLAGS) -M -MG -MQ $@ -o $(BUILD)/dep/$(<:%.c=%.dd) -c $<
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -fpic -fPIC -o $@ -c $< 

$(BUILD)/include/%.h: %.h
	@echo Linking header: $@
	@mkdir -p $(dir $@)
	ln -sf $(abspath $<) $@


# --------------------------------------------------------------------------
# libkmip library targets

LIBKMIP_SHARED = $(BUILD)/lib/libkmip.so.$(LIBKMIP_VER_MAJOR)
LIBKMIP_STATIC = $(BUILD)/lib/libkmip.a

.PHONY: libkmip
libkmip: $(LIBKMIP_SHARED) $(LIBKMIP_STATIC)

LIBKMIP_SOURCES := kmip.c kmip_bio.c kmip_memset.c

$(LIBKMIP_SHARED): $(LIBKMIP_SOURCES:%.c=$(BUILD)/obj/%.lo)
	@echo Building shared library: $@
	@mkdir -p $(dir $@)
	$(CC) -shared -Wl,-soname,libkmip.so.$(LIBKMIP_VER_MAJOR) -o $@ $^ $(LDFLAGS)

$(LIBKMIP_STATIC): $(LIBKMIP_SOURCES:%.c=$(BUILD)/obj/%.o)
	@echo Building static library: $@
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^


# --------------------------------------------------------------------------
# KMIP server validation target

.PHONY: kmip-get
kmip-get: $(BUILD)/bin/kmip-get

$(BUILD)/bin/kmip-get: $(BUILD)/obj/kmip-get.o $(LIBKMIP_STATIC)
	@echo Building executable: $@
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)


# --------------------------------------------------------------------------
# libkmip header targets

.PHONY: headers
headers: $(BUILD)/include/kmip.h $(BUILD)/include/kmip_bio.h $(BUILD)/include/kmip_memset.h


# --------------------------------------------------------------------------
# Test targets

.PHONY: tests
tests: $(BUILD)/bin/tests

$(BUILD)/bin/tests: $(BUILD)/obj/tests.o $(LIBKMIP_STATIC)
	@echo Building executable: $@
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)


# --------------------------------------------------------------------------
# Demonstration targets

.PHONY: demos
demos: $(BUILD)/bin/demo_create $(BUILD)/bin/demo_get $(BUILD)/bin/demo_destroy

$(BUILD)/bin/demo_create: $(BUILD)/obj/demo_create.o $(LIBKMIP_STATIC)
	@echo Building executable: $@
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD)/bin/demo_get: $(BUILD)/obj/demo_get.o $(LIBKMIP_STATIC)
	@echo Building executable: $@
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD)/bin/demo_destroy: $(BUILD)/obj/demo_destroy.o $(LIBKMIP_STATIC)
	@echo Building executable: $@
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)


# --------------------------------------------------------------------------
# Clean target

.PHONY: clean
clean:
	@echo Cleaning: $(BUILD)
	@echo rm -rf $(BUILD)

.PHONY: distclean
distclean:
	@echo Cleaning: $(BUILD)
	rm -rf $(BUILD)


# --------------------------------------------------------------------------
# Clean dependencies target

.PHONY: cleandeps
cleandeps:
	@echo Cleaning dependencies: $(BUILD)/dep
	rm -rf $(BUILD)/dep


# --------------------------------------------------------------------------
# Dependencies

ALL_SOURCES := $(LIBKMIP_SOURCES) kmip-get

$(foreach i, $(ALL_SOURCES), $(eval -include $(BUILD)/dep/$(i:%.c=%.d)))
$(foreach i, $(ALL_SOURCES), $(eval -include $(BUILD)/dep/$(i:%.c=%.dd)))


# --------------------------------------------------------------------------
# tar package target

GIT_TARNAME = libkmip-$(FULL_VERSION)
GIT_TARPATH = $(BUILD)/$(GIT_TARNAME)

dist: $(BUILD) libkmip.spec
	git archive --format=tar --prefix=$(GIT_TARNAME)/ HEAD^{tree} > $(GIT_TARPATH).tar
	mkdir -p $(GIT_TARNAME)
	cp GIT-VERSION-FILE $(GIT_TARNAME)
	cp $(BUILD)/libkmip.spec $(GIT_TARNAME)
	tar -rf $(GIT_TARPATH).tar $(GIT_TARNAME)/GIT-VERSION-FILE $(GIT_TARNAME)/libkmip.spec
	rm -r $(GIT_TARNAME)
	gzip -f -9 $(GIT_TARPATH).tar


# --------------------------------------------------------------------------
# Redhat RPM target --- RPM releases can't have "-"

RPM_RELEASE := $(shell echo $(RELEASE) | tr '-' '.')

%.spec: %.spec.in .FORCE $(BUILD)
	sed -e 's/@@VERSION@@/$(VERSION)/g' \
	    -e 's/@@TAR_NAME@@/$(GIT_TARNAME)/g' \
	    -e 's/@@RELEASE@@/$(RPM_RELEASE)/g' < $< > $@+
	mv $@+ $(BUILD)/$@

RPM_DIR = $(BUILD)/rpmbuild
export RESULT_DIR = $(PWD)/pkg-linux/libkmip-$(FULL_VERSION)

rpm: dist
	@mkdir -p $(RPM_DIR)
	env RPM_DIR=$(RPM_DIR) bash ./pkg-linux/mock_rpmbuild.sh $(GIT_TARPATH).tar.gz $(BUILD)/libkmip.spec

relrpm: dist
	@mkdir -p $(RPM_DIR)
	env RPM_DIR=$(RPM_DIR) REL_BUILD="yes" bash ./pkg-linux/mock_rpmbuild.sh $(GIT_TARPATH).tar.gz $(BUILD)/libkmip.spec

devrpm: dist
	@mkdir -p $(RPM_DIR)
	env RPM_DIR=$(RPM_DIR) bash ./pkg-linux/rpmbuild.sh $(GIT_TARPATH).tar.gz $(BUILD)/libkmip.spec


# --------------------------------------------------------------------------
# Print macros to help in debugging

macros:
	@echo "BUILD:             $(BUILD)"
	@echo "DESTDIR:           $(DESTDIR)"
	@echo "LIBDIR:            $(LIBDIR)"
	@echo ""
	@echo "GIT_TARNAME:       $(GIT_TARNAME)"
	@echo "GIT_TARPATH:       $(GIT_TARPATH)"
	@echo ""
	@echo "LIBKMIP_HEADERS:   $(LIBKMIP_HEADERS)"
	@echo "LIBKMIP_SOURCES:   $(LIBKMIP_SOURCES)"
	@echo "LIBKMIP_SHARED:    $(LIBKMIP_SHARED)"
	@echo "LIBKMIP_STATIC:    $(LIBKMIP_STATIC)"
	@echo ""
	@echo "RPM_DIR:           $(RPM_DIR)"
	@echo "RPM_RELEASE:       $(RPM_RELEASE)"
	@echo ""
	@echo "VERSION:           $(VERSION)"
	@echo "FULL_VERSION:      $(FULL_VERSION)"
	@echo "LIBKMIP_VER:       $(LIBKMIP_VER)"
	@echo "LIBKMIP_VER_MAJOR: $(LIBKMIP_VER_MAJOR)"
	@echo "LIBKMIP_VER_MINOR: $(LIBKMIP_VER_MINOR)"


.PHONY: .FORCE
.FORCE:
