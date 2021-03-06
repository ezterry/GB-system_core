#Build SH unless SKIP_SH_BUILD_USE_BB is true
# then symlink busybox sh

ifneq ($(SKIP_SH_BUILD_USE_BB),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	alias.c \
	arith.c \
	arith_lex.c \
	builtins.c \
	cd.c \
	error.c \
	eval.c \
	exec.c \
	expand.c \
	input.c \
	jobs.c \
	main.c \
	memalloc.c \
	miscbltin.c \
	mystring.c \
	nodes.c \
	options.c \
	parser.c \
	redir.c \
	show.c \
	syntax.c \
	trap.c \
	output.c \
	var.c \
	bltin/echo.c \
	init.c

LOCAL_MODULE:= sh

LOCAL_CFLAGS += -DSHELL -DWITH_LINENOISE

LOCAL_STATIC_LIBRARIES := liblinenoise

LOCAL_C_INCLUDES += system/core/liblinenoise

make_ash_files: PRIVATE_SRC_FILES := $(SRC_FILES)
make_ash_files: PRIVATE_CFLAGS := $(LOCAL_CFLAGS)
make_ash_files:
	p4 edit arith.c arith_lex.c arith.h builtins.h builtins.c 
	p4 edit init.c nodes.c nodes.h token.h 
	sh ./mktokens
	bison -o arith.c arith.y
	flex -o arith_lex.c arith_lex.l
	perl -ne 'print if ( /^\#\s*define\s+ARITH/ );' < arith.c > arith.h
	sh ./mkbuiltins shell.h builtins.def . -Wall -O2
	sh ./mknodes.sh nodetypes nodes.c.pat .
	sh ./mkinit.sh $(PRIVATE_SRC_FILES) 

include $(BUILD_EXECUTABLE)
else

#forget sh just symlink to busybox
file := $(TARGET_OUT)/bin/sh
ALL_PREBUILT += $(file)
$(file) : busybox | $(ACP)
	mkdir -p $(TARGET_OUT)/bin
	ln -sf ../xbin/busybox $(TARGET_OUT)/bin/sh

endif
