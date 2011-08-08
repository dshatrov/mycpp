COMMON_CFLAGS =				\
	-ggdb				\
	-Wno-long-long -Wall -Wextra	\
	-include "config.h"		\
	$(THIS_CFLAGS)

if PLATFORM_WIN32
    COMMON_CFLAGS += -DPLATFORM_WIN32
else
    # For backtraces
    COMMON_CFLAGS += -rdynamic
endif

AM_CFLAGS = -std=gnu99 $(COMMON_CFLAGS)
AM_CXXFLAGS = -std=gnu++0x $(COMMON_CFLAGS) #-fno-default-inline -fkeep-inline-functions

INCLUDES = -I$(top_srcdir)

mycpp_private_headers =			\
	mycpp_thread_local.h

mycpp_target_headers =			\
	mycpp_config.h			\
	mycpp.h				\
					\
	types.h				\
	atomic_int.h			\
	atomic_pointer.h		\
	string.h			\
	memory_desc.h			\
	callback_desc.h			\
	type_info.h			\
					\
	pointer.h			\
	extractor.h			\
	comparator.h			\
	iterator.h			\
	iteratable.h			\
	cloneable.h			\
					\
	simply_referenced.h		\
	referenced.h			\
	object.h			\
	object_factory.h		\
	ref.h				\
	weak_ref.h			\
					\
	list.h				\
	intrusive_list.h		\
	embedded_list.h			\
	avl_tree.h			\
	intrusive_avl_tree.h		\
	map.h				\
	multi_map.h			\
	buffer.h			\
	linear_buffer_info.h		\
	vstack.h			\
	vslab.h				\
					\
	array.h				\
	array_holder.h			\
	smart_array_holder.h		\
	external_array.h		\
	block_list.h			\
	dynamic_tree_array.h		\
					\
	mutex.h				\
	data_mutex.h			\
	cond.h				\
	rw_lock.h			\
					\
	exception.h			\
	internal_exception.h		\
	io_exception.h			\
	middleware_exception.h		\
					\
	informer.h			\
					\
	trigger.h			\
	immediate_trigger.h		\
	scheduler_trigger.h		\
					\
	file.h				\
	async_file.h			\
	stream_in.h			\
	stream_out.h			\
	async_stream_in.h		\
	async_stream_out.h		\
	file_stream_in.h		\
	file_stream_out.h		\
	file_async_stream_in.h		\
	file_async_stream_out.h		\
	async_stream_pump.h		\
	array_file.h			\
	direct_array_file.h		\
	native_file.h			\
	cpp_file.h			\
	cached_file.h			\
	print_task.h			\
	io.h				\
					\
	vfs.h				\
					\
	io_subject.h			\
	io_actor.h			\
	pollable.h			\
	poll_group.h			\
	active_poll_group.h		\
	busy_poll_group.h		\
	poller.h			\
	background_poller.h		\
	simple_threaded_poller.h	\
					\
	thread.h			\
	branch.h			\
	deletion_queue.h		\
	scheduler.h			\
	immediate_scheduler.h		\
	thread_pool.h			\
	thread_group.h			\
	poll_group_scheduler.h		\
	threaded_poll.h			\
					\
	action_boss.h			\
					\
	id_mapper.h			\
	regex.h				\
	cmdline.h			\
	uid.h				\
					\
	base_util.h			\
	mem_util.h			\
	strutil.h			\
	convert.h			\
	unicode.h			\
	utf8.h				\
	util.h				\
					\
	synchronization_test.h		\
	mem_prof_link.h			\
	mem_prof.h			\
	debug.h

# Trash
#	enumeration.h
#
#	waitable.h
#	wait_group.h
#	call.h
#	completion_notifier.h
#
#	tree.h
#	page_pool.h
#	data_cache.h
#
#	buffered_file.h
#	direct_file.h

mycpp_linux_target_headers =		\
	vfs_posix.h			\
					\
	native_async_file.h		\
	select_poll_group.h		\
	epoll_poll_group.h		\
					\
	posix.h				\
	native_file.linux.h		\
					\
	sync_pollable.h

mycpp_win32_target_headers =		\
	win32.h				\
	native_file.win32.h		\
	wsa_poll_group.h
#	threaded_wsa_poll_group.h

if PLATFORM_WIN32
    mycpp_target_headers  += $(mycpp_win32_target_headers)
    mycpp_private_headers += $(mycpp_linux_target_headers)
else
    mycpp_target_headers  += $(mycpp_linux_target_headers)
    mycpp_private_headers += $(mycpp_win32_target_headers)
endif

MYCPP_GENFILES =	\
	native_file.h
BUILT_SOURCES = $(MYCPP_GENFILES)
CLEANFILES = $(MYCPP_GENFILES)
nodist_libmycpp_1_0_la_SOURCES = $(MYCPP_GENFILES)

if PLATFORM_WIN32
native_file.h: native_file.win32.h
	ln -s native_file.win32.h native_file.h
else
native_file.h: native_file.linux.h
	ln -s native_file.linux.h native_file.h
endif

libmycppincludedir = $(includedir)/mycpp-1.0/mycpp
libmycppinclude_HEADERS = $(mycpp_target_headers)

lib_LTLIBRARIES = libmycpp-1.0.la
libmycpp_1_0_la_SOURCES =		\
	informer.cpp			\
	cached_file.cpp			\
					\
	mycpp.cpp			\
	util.cpp			\
	strutil.cpp			\
	unicode.cpp			\
	utf8.cpp			\
	c_util.c			\
	convert.cpp			\
	exception.cpp			\
	cmdline.cpp			\
					\
	embedded_list.cpp		\
					\
	mutex.cpp			\
	data_mutex.cpp			\
	cond.cpp			\
	rw_lock.cpp			\
					\
	simply_referenced.cpp		\
	referenced.cpp			\
	object.cpp			\
	type_info.cpp			\
	atomic_int.cpp			\
	atomic_pointer.cpp		\
	string.cpp			\
	buffer.cpp			\
					\
	id_mapper.cpp			\
	trigger.cpp			\
	immediate_trigger.cpp		\
	scheduler_trigger.cpp		\
	uid.cpp				\
					\
	vfs.cpp				\
					\
	page_pool.cpp			\
					\
	completion_notifier.cpp		\
					\
	io_actor.cpp			\
	pollable.cpp			\
	active_poll_group.cpp		\
	busy_poll_group.cpp		\
	poller.cpp			\
	background_poller.cpp		\
	simple_threaded_poller.cpp	\
					\
	waitable.cpp			\
	wait_group.cpp			\
					\
	mycpp_thread_local.cpp		\
	thread.cpp			\
	branch.cpp			\
	deletion_queue.cpp		\
	immediate_scheduler.cpp		\
	thread_pool.cpp			\
	thread_group.cpp		\
	poll_group_scheduler.cpp	\
	threaded_poll.cpp		\
	file.cpp			\
	async_file.cpp			\
					\
	array_file.cpp			\
	direct_array_file.cpp		\
	cpp_file.cpp			\
	async_stream_in.cpp		\
	async_stream_out.cpp		\
	async_stream_pump.cpp

if ENABLE_MEMPROF
libmycpp_1_0_la_SOURCES +=		\
	mem_prof.cpp
endif

#	regex.cpp
# TODO Add --enable-regex option to ./configure

#	cached_file.cpp

mycpp_linux_sources =			\
	vfs_posix.cpp			\
	native_async_file.cpp		\
	select_poll_group.cpp		\
	epoll_poll_group.cpp		\
					\
	posix.cpp			\
	native_file.linux.cpp		\
					\
	sync_pollable.cpp

mycpp_win32_sources =			\
	win32.cpp			\
	native_file.win32.cpp		\
	wsa_poll_group.cpp
#	threaded_wsa_poll_group.cpp

mycpp_extra_dist =

if PLATFORM_WIN32
    libmycpp_1_0_la_SOURCES += $(mycpp_win32_sources)
    mycpp_extra_dist += $(mycpp_linux_sources)
else
    libmycpp_1_0_la_SOURCES += $(mycpp_linux_sources)
    mycpp_extra_dist += $(mycpp_win32_sources)
endif

#	file_stream_in.cpp		\
#	file_stream_out.cpp		\
#	file_async_stream_in.cpp	\
#	file_async_stream_out.cpp

mycpp_extra_dist +=		\
	buffered_file.cpp	\
	direct_file.cpp		\
				\
	data_cache.cpp

libmycpp_1_0_la_LDFLAGS = -no-undefined -version-info "0:0:0"
libmycpp_1_0_la_LIBADD = $(THIS_LIBS)
if PLATFORM_WIN32
libmycpp_1_0_la_LIBADD += -lws2_32
endif

EXTRA_DIST = $(mycpp_private_headers) $(mycpp_extra_dist)

