noinst_LTLIBRARIES = libinternal.la

libinternal_la_SOURCES =        \
    interface_adaptor.h         \
    restic.h    restic.cpp      \
    rsync.h     rsync.cpp       \
    SystemCmd.h SystemCmd.cpp

libinternal_la_LIBADD = ../utils/libutils.la -lboost_thread -lboost_system
