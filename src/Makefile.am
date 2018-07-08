## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libghwp.la

NOINST_H_FILES = 

INST_H_FILES =             \
	ghwp.h             \
	ghwp-document.h    \
	ghwp-file.h        \
	ghwp-models.h      \
	ghwp-page.h        \
	ghwp-parse.h       \
	ghwp-section.h     \
	ghwp-version.h     \
	gsf-input-stream.h \
	ghwp-file-v3.h     \
	ghwp-file-v5.h     \
	ghwp-file-ml.h     \
	ghwp-context-v3.h  \
	hnc2unicode.h

ghwpincludedir = $(includedir)/ghwp
ghwpinclude_HEADERS = $(INST_H_FILES)

libghwp_la_SOURCES =       \
	ghwp.c             \
	ghwp-document.c    \
	ghwp-file.c        \
	ghwp-models.c      \
	ghwp-page.c        \
	ghwp-parse.c       \
	ghwp-section.c     \
	gsf-input-stream.c \
	ghwp-file-v3.c     \
	ghwp-file-v5.c     \
	ghwp-file-ml.c     \
	ghwp-context-v3.c  \
	hnc2unicode.c      \
	$(NOINST_H_FILES)  \
	$(INST_H_FILES)


libghwp_la_CPPFLAGS =       \
	-DG_LOG_DOMAIN=\"GHWP\" \
	-DGHWP_COMPILATION      \
	$(AM_CPPFLAGS)

libghwp_la_CFLAGS = \
	$(GHWP_CFLAGS)  \
	-Wall           \
	$(AM_CFLAGS)

libghwp_la_LDFLAGS =                      \
	-version-info $(GHWP_LT_VERSION_INFO) \
	-no-undefined                         \
	-export-symbols-regex "^ghwp_*"       \
	$(AM_LDFLAGS)

libghwp_la_LIBADD = $(GHWP_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ghwp-0.2.pc

EXTRA_DIST =       \
	ghwp-0.2.pc.in \
	ghwp-version.h.in

DISTCLEANFILES = \
	ghwp-0.2.pc  \
	ghwp-version.h
