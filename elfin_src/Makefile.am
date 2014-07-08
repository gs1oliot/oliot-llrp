lib_LTLIBRARIES = libelfin.la
include_HEADERS = ELFINReader.h ELFIN_Defs.h ELFIN_ProgOpts.h Stubs/StubApp.h Stubs/StubTag.h
libelfin_la_SOURCES = \
	AbstractAntennaOperation.h AbstractAntennaOperation.cpp	\
	AccessOperation.h AccessOperation.cpp	\
	AIOperation.h AIOperation.cpp	\
	AOAdmin.h AOAdmin.cpp	\
	CConnectionFnC.h CConnectionFnC.cpp	\
	CConnectionFnCMgr.h CConnectionFnCMgr.cpp	\
	ELFIN_Defs.h	 \
	ELFIN_Platform.h ELFIN_Platform.cpp	\
	ELFIN_ProgOpts.h ELFIN_ProgOpts.cpp	\
	ELFINReader.h ELFINReader.cpp	\
	EventNotifier.h EventNotifier.cpp	\
	GPIEventListener.h GPIEventListener.cpp	\
	LLRPCore.h LLRPCore.cpp	\
	LLRPReader_Configurations.h LLRPReader_Configurations.cpp	\
	ReaderHandler/AbstractReader.h ReaderHandler/AbstractReader.cpp	\
	ReaderHandler/ReaderCapabilities.h 	\
	ReaderHandler/SampleReader.h ReaderHandler/SampleReader.cpp	\
	ReaderOperation.h ReaderOperation.cpp	\
	RFSurveyOperation.h RFSurveyOperation.cpp	\
	ROAdmin.h ROAdmin.cpp	\
	Scheduler.h Scheduler.cpp	\
	Stubs/StubAntenna.h Stubs/StubAntenna.cpp	\
	Stubs/StubApp.h Stubs/StubApp.cpp	\
	Stubs/StubGPIPort.h Stubs/StubGPIPort.cpp	\
	Stubs/StubGPOPort.h Stubs/StubGPOPort.cpp	\
	Stubs/StubReader.h Stubs/StubReader.cpp	\
	Stubs/StubTag.h Stubs/StubTag.cpp	\
	TimerTask.h TimerTask.cpp	\
	Triggers/AbstractTrigger.h 	\
	Triggers/ROSpecTrigger.h Triggers/ROSpecTrigger.cpp
#libelfin_la_LIBADD = -lboost_thread -lboost_system -lboost_program_options \
#	$(top_srcdir)/ThirdParty/LTK/LTKCPP/Library/libltkcpp.a
libelfin_la_LIBADD = \
	/usr/local/lib/libboost_program_options.so \
	/usr/local/lib/libboost_thread.so \
	/usr/local/lib/libboost_system.so \
	/usr/local/lib/libboost_program_options.so \
	$(top_srcdir)/ThirdParty/libxml2/libxml2.la \
	$(top_srcdir)/ThirdParty/LTK/LTKCPP/Library/libltkcpp.a
#libelfin_la_LDFLAGS = -version-info 1.0.0
INCLUDES = -I$(top_srcdir)/ThirdParty/LTK/LTKCPP/Library -I/usr/local/include
libelfin_la_CXXFLAGS = -O2 -g -Wall -fPIC
libelfin_la_DEPENDENCIES = $(top_srcdir)/ThirdParty/LTK/LTKCPP/Library/libltkcpp.a
