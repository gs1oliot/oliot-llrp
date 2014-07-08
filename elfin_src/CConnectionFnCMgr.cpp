/**
 * @file    CConnectionFnCMgr.cpp
 * @brief
 */

#include <exception>
#include <stdio.h>
#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "CConnectionFnCMgr.h"

int ELFIN::CConnectionFnCMgr::_isConnected = 0;

ELFIN::CConnectionFnCMgr::CConnectionFnCMgr() {
	_keepaliveThread = NULL;
	_receivedKeepaliveID = -1;
	_isConnected = 0;
	_portNum = 0;
	_isReaderInitiated = 0;
	_kaStop = 0;
	_pConn = NULL;
	_pTypeRegistry = NULL;
}

ELFIN::CConnectionFnCMgr::~CConnectionFnCMgr(void) {
	READER_LOG (LOGTYPE_TRACE, "Destroying CConnectionFnCMgr...\n");
	_isConnected = 0;
	if (_keepaliveThread != NULL) {
		_kaStop = 0;
		_keepaliveThread->interrupt();
		_keepaliveThread->join();
		delete _keepaliveThread;
		_keepaliveThread = NULL;
	}
	_sendMtx.lock();
	if (_pConn != NULL) {
		//closeConnectionToFnC(0);
		delete _pConn;
		_pConn = NULL;
	}

	if (_pTypeRegistry != NULL) {
		delete _pTypeRegistry;
		_pTypeRegistry = NULL;
	}

	_sendMtx.unlock();
}

int ELFIN::CConnectionFnCMgr::startConnection(ProgramOptions *_progOpt) {
	LLRP::CTypeRegistry *             pTypeRegistry;

	int                         rc;

	/*
	 * Allocate the type registry. This is needed
	 * by the connection to decode.
	 */
	pTypeRegistry = LLRP::getTheTypeRegistry();
	if(NULL == pTypeRegistry)
	{
		READER_LOG(LOGTYPE_ERROR, "getTheTypeRegistry failed\n");
		//return -1;
	}
	//Enrollment of LLRP 1.1 Extension to the type registry
	//enrollLLRP1x1ExtTypesIntoRegistry(pTypeRegistry);

	_pTypeRegistry = pTypeRegistry;
	/*
	 * Construct a connection (LLRP::CConnection).
	 * Using a 32kb max frame size for send/recv.
	 * The connection object is ready for business
	 * but not actually connected to the reader yet.
	 */
	if (_pConn != NULL) {
		READER_LOG(LOGTYPE_ERROR, "_pConn is not NULL\n");
	}

	_portNum = _progOpt->getFnCPort();

	_pConn = new CConnectionFnC(pTypeRegistry, CCONNECTION_BUFFER_SIZE, _portNum);
	if(NULL == _pConn)
	{
		READER_LOG(LOGTYPE_ERROR, "new CConnection failed\n");
		rc = -2;
		//return -2;
	}
	_isReaderInitiated = _progOpt->getReaderInitiatedOpt();
	if (_isReaderInitiated == 1) {
		std::string FnCHostName = _progOpt->getFnCAddress();
		READER_LOG(LOGTYPE_INFO, "LLRP Reader starts in client mode....\n");
		rc = openConnectionToFnC(FnCHostName.c_str());
	} else if (_isReaderInitiated == -1) {
		exit(0);
	} else {
		READER_LOG(LOGTYPE_INFO, "LLRP Reader starts in server mode....\n");
		READER_LOG(LOGTYPE_INFO, "Start waiting connection....\n");
		rc = waitForConnection();
	}
	if (rc == 0) {
		READER_LOG(LOGTYPE_INFO, "Connected, checking status....\n");
		_isConnected = 1;
		sendConnectionAttemptEvent();
	}
	else {
		READER_LOG(LOGTYPE_INFO, "Connection failed\n");
		delete _pTypeRegistry;
		_pTypeRegistry = NULL;
		delete _pConn;
		_pConn = NULL;
		_isConnected = 0;
	}
	return rc;
}


int ELFIN::CConnectionFnCMgr::waitForConnection() {
	_sendMtx.lock();
	if (_pConn == NULL)
		return -1;
	int rc = _pConn->acceptConnectionFromClient();
	_sendMtx.unlock();
	if(0 != rc)
	{
		READER_LOG(LOGTYPE_ERROR, "connect: %s (%d)\n", _pConn->getConnectError(), rc);
		delete _pConn;
		return -3;
	}
	return 0;
}

int ELFIN::CConnectionFnCMgr::sendConnectionAttemptEvent() {
	LLRP::CREADER_EVENT_NOTIFICATION		*pNtf;
	LLRP::CReaderEventNotificationData	*pNtfData;

	LLRP::CUTCTimestamp					*pTmStamp;
	LLRP::CConnectionAttemptEvent			*pEvent;


	//Create and set ConnectionAttemptStatus
	pEvent = new LLRP::CConnectionAttemptEvent();
	pEvent->setStatus(LLRP::ConnectionAttemptStatusType_Success);

	pTmStamp = ELFIN::Utils::getCurrentUTCTimestamp();

	//Create and set ReaderEventNotification
	pNtfData = new LLRP::CReaderEventNotificationData();

	//Time stamp
	pNtfData->setTimestamp(pTmStamp);
	//Connection Attempt
	pNtfData->setConnectionAttemptEvent(pEvent);



	/*
	 * Compose the command message.
	 * N.B.: After the message is composed, all the parameters
	 *       constructed, immediately above, are considered "owned"
	 *       by the command message. When it is destructed so
	 *       too will the parameters be.
	 */
	pNtf = new LLRP::CREADER_EVENT_NOTIFICATION();
	pNtf->setMessageID(0);
	pNtf->setReaderEventNotificationData(pNtfData);

	/*
	 * Send the message, expect the response of certain type
	 */
	//EResultCode rc = _pConn->sendMessage(pNtf);
	READER_PRINTXMLMSG(pNtf);
	//_pConn->sendMessage(pNtf);
	this->sendMessage(pNtf);

	delete pNtf;

	/*
	 * Victory.
	 */
	return 0;
}



int ELFIN::CConnectionFnCMgr::sendConnectionCloseEvent() {
	LLRP::CREADER_EVENT_NOTIFICATION		*pNtf;
	LLRP::CReaderEventNotificationData	*pNtfData;

	LLRP::CUTCTimestamp					*pTmStamp;
	LLRP::CConnectionCloseEvent			*pEvent;


	//Create and set ConnectionAttemptStatus
	pEvent = new LLRP::CConnectionCloseEvent();

	pTmStamp = ELFIN::Utils::getCurrentUTCTimestamp();

	//Create and set ReaderEventNotification
	pNtfData = new LLRP::CReaderEventNotificationData();

	//Time stamp
	pNtfData->setTimestamp(pTmStamp);
	//Connection Attempt
	pNtfData->setConnectionCloseEvent(pEvent);



	/*
	 * Compose the command message.
	 * N.B.: After the message is composed, all the parameters
	 *       constructed, immediately above, are considered "owned"
	 *       by the command message. When it is destructed so
	 *       too will the parameters be.
	 */
	pNtf = new LLRP::CREADER_EVENT_NOTIFICATION();
	pNtf->setMessageID(0);
	pNtf->setReaderEventNotificationData(pNtfData);

	/*
	 * Send the message, expect the response of certain type
	 */
	//EResultCode rc = _pConn->sendMessage(pNtf);
	//READER_PRINTXMLMSG(pNtf);
	//_pConn->sendMessage(pNtf);
	this->sendMessage(pNtf);

	delete pNtf;

	/*
	 * Victory.
	 */
	return 0;
}


LLRP::CMessage* ELFIN::CConnectionFnCMgr::recvMessage(int nMaxMS) {
	boost::recursive_mutex::scoped_lock lock(_sendMtx);
	if (this->_isConnected == 0)
		return NULL;
	LLRP::CMessage* pMessage = _pConn->recvMessage(nMaxMS);
	if (pMessage != NULL)
		READER_PRINTXMLMSG(pMessage);

	return pMessage;
}

const LLRP::CErrorDetails *ELFIN::CConnectionFnCMgr::getRecvError (void) {
	return this->_pConn->getRecvError();
}

int ELFIN::CConnectionFnCMgr::openConnectionToFnC(
		const char* pReaderHostName) {
	_sendMtx.lock();
	if (_pConn == NULL)
		return -1;
	int result = _pConn->openConnectionToFnC (pReaderHostName);
	_sendMtx.unlock();
	return result;
}

/* Send LLRP Message to the connected FnC Middleware.
 * Modify this method if you want to send message to other applications.
 */
int ELFIN::CConnectionFnCMgr::sendMessage(LLRP::CMessage* pMessage)
{
	int result;
	boost::recursive_mutex::scoped_lock lock(_sendMtx);
	if (this->isConnected() == 0) {
		READER_LOG(LOGTYPE_ERROR, "Connection is already closed. Failed to send message %s\n", pMessage->m_pType->m_pName);
		return -1;
	}
	if (pMessage == NULL) {
		READER_LOG(LOGTYPE_ERROR, "NULL message is given as parameter to sendMessage().\n");
		return -1;
	}
	READER_LOG(LOGTYPE_DEBUG, "Sending message...: %s\n", pMessage->m_pType->m_pName);
	READER_PRINTXMLMSG(pMessage);

	if (_pConn == NULL) {
		READER_LOG(LOGTYPE_ERROR, "Connection is already closed. Failed to send message %s\n", pMessage->m_pType->m_pName);
		return -1;
	}
	result = _pConn->sendMessage(pMessage);

	if (result != 0) {
		const LLRP::CErrorDetails *errDetails = _pConn->getSendError();
		READER_LOG(LOGTYPE_ERROR, "Connection is closed from the server: %s\n",
				errDetails->m_pWhatStr);
		closeConnectionToFnC(_isReaderInitiated);
		return -1;
	}
	return result;
}

const char* ELFIN::CConnectionFnCMgr::getConnectError(void) {
	return _pConn->getConnectError();
}

int ELFIN::CConnectionFnCMgr::getReceivedKeepaliveId() {
	boost::mutex::scoped_lock lock(_kaMtx);
	int val = _receivedKeepaliveID;
	return val;
}

int ELFIN::CConnectionFnCMgr::closeConnectionToFnC(int __isReaderInitiated) {
	int result;
	_sendMtx.lock();
	if (_isConnected == 0)
		return 0;
	_isConnected = 0;
	if (_pConn == NULL)
		return -1;
	result = _pConn->closeConnectionToFnC(__isReaderInitiated);
	READER_LOG(LOGTYPE_ERROR, "Connection close status: %d\n", result);
	delete _pConn;
	_pConn = NULL;
	_sendMtx.unlock();
	return result;
}

void ELFIN::CConnectionFnCMgr::setReceivedKeepaliveId(
		int __receivedKeepaliveID) {
	boost::mutex::scoped_lock lock(_kaMtx);
	_receivedKeepaliveID = __receivedKeepaliveID;
}

int ELFIN::CConnectionFnCMgr::startKeepaliveThread(int periodMS) {
	if (_keepaliveThread != NULL) {
		READER_LOG(LOGTYPE_INFO, "KeepaliveThread is already running. Trying to join...\n");
		_keepaliveThread->join();
		delete _keepaliveThread;
		_keepaliveThread = NULL;
	}
	READER_LOG(LOGTYPE_INFO, "Starting KeepaliveThread...\n");
	_kaStop = 0;
	_keepaliveThread = new boost::thread(
			boost::bind(&CConnectionFnCMgr::keepalive_run, this, periodMS));
	return 0;
}

int ELFIN::CConnectionFnCMgr::stopKeepaliveThread() {
	READER_LOG(LOGTYPE_INFO, "Stopping KeepaliveThread...\n");
	_kaStop = 1;
	_keepaliveThread->interrupt();
	_keepaliveThread->join();
	return 0;
}

void ELFIN::CConnectionFnCMgr::keepalive_run(int periodMS) {
	int kaFailCount = 0;
	for (;;) {
		if (_kaStop == 1)
			return;
		LLRP::CKEEPALIVE *kaMsg = new LLRP::CKEEPALIVE();
		int result = sendMessage(kaMsg);
		delete kaMsg;
		if (result != 0) {
			READER_LOG(LOGTYPE_ERROR, "Failed to send keepalive message\n");
			return;
		}
		this->setReceivedKeepaliveId(-1);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(periodMS));
		int kaID = this->getReceivedKeepaliveId();
		if (kaID == -1) {
			kaFailCount++;
			READER_LOG(LOGTYPE_ERROR, "Keepalive failed...%d\n", kaFailCount);
			if (kaFailCount >= KEEPALIVE_FAIL_LIMIT) {
				READER_LOG(LOGTYPE_ERROR, "Connection would be closed because of keepalive failure\n");
				closeConnectionToFnC(_isReaderInitiated);
				return;
			}
		}
	}
}

int ELFIN::CConnectionFnCMgr::isConnected() {
	return _isConnected;
}
