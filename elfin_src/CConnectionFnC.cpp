/**
 * @file    CConnectionFnC.cpp
 * @brief
 */

#include <exception>

#include <assert.h>

#if defined(linux) || defined(__APPLE__)
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#endif
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#endif
#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

//#include "LLRPCore.h"
#include "CConnectionFnC.h"

//#define LLRP1_TCP_PORT   (5084u)
//#define LLRP1_TCP_PORT_STR   "5084"

/*
 * On Linux a socket is a simple type (int). On Windows it
 * is a pointer to a specific type defined in a WinSock
 * header (.h) file. Rather than make that header file a
 * prerequisite to every source file that includes this header
 * file (ltkcpp_connection.h), the following CPlatformSocket
 * class opaquely wraps the platform-specific socket.
 *
 * The CConnection class references it by pointer only.
 * The content of the CPlatformSocket is only known
 * within the implementation of CConnection.
 */

ELFIN::CConnectionFnC::CConnectionFnC(const LLRP::CTypeRegistry* pTypeRegistry,
		unsigned int nBufferSize, unsigned int portNum)
:LLRP::CConnection(pTypeRegistry, nBufferSize, portNum) {

}

ELFIN::CConnectionFnC::~CConnectionFnC(void) {
	READER_LOG (LOGTYPE_TRACE, "Destroying CConnectionFnC...\n");
}

/** @remark If __isReaderInitiated = 0, then close listening socket and communication socket.\n
 * If not, just close communication socket.\n
 * The method that closes communication socket is closeConnectionToReader(adopted from LTK),
 * but what it does is just closing the socket.
 */
int ELFIN::CConnectionFnC::closeConnectionToFnC(int __isReaderInitiated) {
	// Modified LTKCPP to check whether this is initiated/accepted connection.
	// So, ignore __isReaderInitiated parameter.
	return this->closeConnection();
}

int ELFIN::CConnectionFnC::openConnectionToFnC(const char* pReaderHostName) {
	return this->openConnection(pReaderHostName);
}


int ELFIN::CConnectionFnC::acceptConnectionFromClient() {
	int result = this->acceptConnection();
	if (result == 0) {
		READER_LOG(LOGTYPE_INFO, "Waiting for the client....\n");
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "Failed to start connection....\n");
	}
	return result;
}
