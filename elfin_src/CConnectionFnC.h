/**
 * @file    CConnectionFnC.h
 * @brief
 */

#ifndef __LLRP_READER__CCONNECTIONFNC_H__
#define __LLRP_READER__CCONNECTIONFNC_H__

#include <vector>
// #include "LLRPCore.h"
#include <boost/asio.hpp>
#include "ELFIN_Platform.h"

//#define LLRP1_TCP_PORT_STR   "5084"

namespace ELFIN {
class LLRPCore;
// class CConnection;
class CConnectionFnC;
}

namespace ELFIN {
/** @class CConnectionFnC
 * @brief Inherits LLRP::CConnection class. It has additional methods for Reader role.
 * @remark Additional methods are moved to CConnection of LTK. Now, this class just acts
 * similar to the proxy of LLRP::CConnection class.
 */
class CConnectionFnC: public LLRP::CConnection {
public:
	/// Constructor of CConnectionFnC class
	CConnectionFnC(const LLRP::CTypeRegistry * pTypeRegistry,
			unsigned int nBufferSize, unsigned int portNum);
	/// Destructor of CConnectionFnC class
	~CConnectionFnC(void);
	/// Close connection to the FnC server
	int closeConnectionToFnC(int __isReaderInitiated);
	/// Initiate connection to the FnC server (client mode)
	int openConnectionToFnC(const char* pReaderHostName);
	/// Wait for connection from the FnC server (server mode)
	int acceptConnectionFromClient();
};
}

#endif /* __LLRP_READER__CCONNECTIONFNC_H__ */
