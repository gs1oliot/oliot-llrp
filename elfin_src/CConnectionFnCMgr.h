/**
 * @file    CConnectionFnCMgr.h
 * @brief
 */

#ifndef __LLRP_READER__CCONNECTIONFNCMGR_H__
#define __LLRP_READER__CCONNECTIONFNCMGR_H__

#include <boost/thread.hpp>
#include "CConnectionFnC.h"
#include "ELFIN_Platform.h"
#include "ELFIN_ProgOpts.h"

#define LLRP1_TCP_PORT_STR   "5084"

namespace ELFIN
{
// class CConnection;
class CConnectionFnC;
}

namespace ELFIN
{
/** @class CConnectionFnCMgr
 * @brief Inherits CConnection class. It has additional methods for Reader role.
 */
class CConnectionFnCMgr
{
public:
	/// Constructor of CConnectionFnCMgr class
	CConnectionFnCMgr();
	/// Destructor of CConnectionFncMgr class
	~CConnectionFnCMgr (void);
	/// Close connection to the FnC server
	int closeConnectionToFnC(int __isReaderInitiated);
	/// Send ConnectionAttemptEvent notification to the FnC server
	int sendConnectionAttemptEvent();
	/// Send ConnectionCloseEvent notification to the FnC server
	int sendConnectionCloseEvent();
	/// Create the keepalive thread
	int startKeepaliveThread(int periodMS);
	/// Interrupt and stop the keepalive thread
	int stopKeepaliveThread();
	/// This method is used to make thread for keepalive. Send KEEPALIVE message and check the KEEPALIVE_ACK periodically.
	void keepalive_run(int periodMS);

	/// Start connection to the FnC server. Checks reader_initiated option to decide whether to initiate connection or wait for connection.
	int startConnection(ProgramOptions *__progOpt);
	/// Receive the message in recv buffer
	LLRP::CMessage* recvMessage(int nMaxMS);
	/// Queue the message to send buffer
	int sendMessage(LLRP::CMessage* pMessage);
	/// Get currently stored error string
	const char *getConnectError (void);
	/// Get currenlty stored CErrorDetails
	const LLRP::CErrorDetails *getRecvError (void);

	/// Returns recently received keepalive id
	int getReceivedKeepaliveId();
	/// Set the _receivedKeepaliveID id with given id
	void setReceivedKeepaliveId(int __receivedKeepaliveID);
	boost::thread *_keepaliveThread;
	/// Check whether the LLRP Wapper is connected to FnC server
	int isConnected();

private:
	boost::recursive_mutex _sendMtx;
	CConnectionFnC* _pConn;
	static int _isConnected;
	int _portNum;
	int _receivedKeepaliveID;
	boost::mutex _kaMtx;
	int _kaStop;
	int _isReaderInitiated;
	LLRP::CTypeRegistry *_pTypeRegistry;

	/// Initiate connection to the FnC server (client mode)
	int openConnectionToFnC(const char *pReaderHostName);
	/// Wait for connection from the FnC server (server mode)
	int waitForConnection();
};
}


#endif /* __LLRP_READER__CCONNECTIONFNCMGR_H__ */
