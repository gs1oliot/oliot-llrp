bin_PROGRAMS = elfin_app

elfin_app_SOURCES = 		\
	ReaderApp.h	\
	ReaderApp.cpp

elfin_app_LDADD   = -lelfin -lxml2 -lboost_thread -lboost_system -lboost_program_options
elfin_app_LDFLAGS = -L/usr/local/lib -L$(top_builddir)/elfin_src

INCLUDES = -I$(top_srcdir)/elfin_src -I/usr/local/include
elfin_app_CXXFLAGS = -O2 -g -Wall -fPIC
elfin_app_DEPENDENCIES = $(top_builddir)/elfin_src/libelfin.la