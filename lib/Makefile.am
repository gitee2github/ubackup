SUBDIRS = utils internal  

include_HEADERS = ubackup.h

lib_LTLIBRARIES = libubackup.la

libubackup_la_SOURCES = ubackup.h interface.cpp \
                configuration.h    configuration.cpp    

libubackup_la_LIBADD = internal/libinternal.la
libubackup_la_LDFLAGS = -lboost_filesystem -lboost_system
