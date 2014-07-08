/*
 * TCR_R1.h
 *
 *  Created on: Aug 31, 2013
 *      Author: shheo
 */

#ifndef TCR_R1_H_
#define TCR_R1_H_

#include <boost/thread.hpp>
#include "LLRPTestClient.h"

#define CONST_R1 10000

class TCR_R1 {
public:

	TCR_R1 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient),
		m_LLRPConfigurationStateValue(0) {};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;
	llrp_u32_t m_LLRPConfigurationStateValue;

	int TCR_R1_Step1 (void);
	int TCR_R1_Step2 (void);
	int TCR_R1_Step3 (void);
	int TCR_R1_Step4 (void);
	int TCR_R1_Step5 (void);
	int TCR_R1_Step6 (void);
	int TCR_R1_Step7 (void);
};

inline int TCR_R1::TCR_R1_Step1 (void) {
	CMessage *                  pMessage;
	CREADER_EVENT_NOTIFICATION *pNtf;
	CReaderEventNotificationData *pNtfData;
	CConnectionAttemptEvent *   pEvent;

	if (_pTestClient->waitconenctionFromReader() != 0)
		goto fail;

	/*
	 * Expect the notification within 10 seconds.
	 * It is suppose to be the very first message sent.
	 */
	pMessage = _pTestClient->recvMessage(10000);

	/*
	 * recvMessage() returns NULL if something went wrong.
	 */
	if(NULL == pMessage)
	{
		/* recvMessage already tattled */
		goto fail;
	}

	/*
	 * Check to make sure the message is of the right type.
	 * The type label (pointer) in the message should be
	 * the type descriptor for READER_EVENT_NOTIFICATION.
	 */
	if(&CREADER_EVENT_NOTIFICATION::s_typeDescriptor != pMessage->m_pType)
	{
		goto fail;
	}

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pNtf = (CREADER_EVENT_NOTIFICATION *) pMessage;
	pNtfData = pNtf->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	/*
	 * The ConnectionAttemptEvent parameter must be present.
	 */
	pEvent = pNtfData->getConnectionAttemptEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	/*
	 * The status in the ConnectionAttemptEvent parameter
	 * must indicate connection success.
	 */
	if(ConnectionAttemptStatusType_Success != pEvent->getStatus())
	{
		goto fail;
	}

	/*
	 * Done with the message
	 */
	delete pMessage;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R1_Step1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	/*
	 * Victory.
	 */
	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: checkConnectionStatus failed\n");
	delete pMessage;
	return -1;
}

inline int TCR_R1::TCR_R1_Step2(void) {
	CGET_READER_CONFIG *        pCmd;
	CMessage *                  pRspMsg;
	CGET_READER_CONFIG_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CGET_READER_CONFIG();
	pCmd->setMessageID(121);
	pCmd->setRequestedData((enum EGetReaderConfigRequestedData) 7);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = _pTestClient->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CGET_READER_CONFIG_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "getLLRPConfigurationStateValue"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	m_LLRPConfigurationStateValue = pRsp->getLLRPConfigurationStateValue()->getLLRPConfigurationStateValue();

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R1_Step2 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R1::TCR_R1_Step3(void) {
	LLRPTestClient *tempTestClient = new LLRPTestClient(_pReaderHostName);
	tempTestClient->openConnectionToReader();
	delete tempTestClient;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R1_Step3 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R1::TCR_R1_Step4(void) {
	CCLOSE_CONNECTION *        pCmd;
	CMessage *                  pRspMsg;
	CCLOSE_CONNECTION_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CCLOSE_CONNECTION();
	pCmd->setMessageID(121);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = _pTestClient->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CCLOSE_CONNECTION_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "getLLRPConfigurationStateValue"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/* Wait for R1 (Default = 10) seconds */
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R1));

	/* Resend CLOSE_CONNECTION to verify the connection is closed */
	pCmd = new CCLOSE_CONNECTION();
	pCmd->setMessageID(121);
	pRspMsg = _pTestClient->transact(pCmd);
	delete pCmd;
	if(NULL != pRspMsg)
	{
		/* connection is not closed, so fail */
		return -1;
	}

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R1_Step4 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R1::TCR_R1_Step5(void) {
	CMessage *                  pMessage;
	CREADER_EVENT_NOTIFICATION *pNtf;
	CReaderEventNotificationData *pNtfData;
	CConnectionAttemptEvent *   pEvent;

	if (_pTestClient->openConnectionToReader() != 0)
		goto fail;

	/*
	 * Expect the notification within 10 seconds.
	 * It is suppose to be the very first message sent.
	 */
	pMessage = _pTestClient->recvMessage(10000);

	/*
	 * recvMessage() returns NULL if something went wrong.
	 */
	if(NULL == pMessage)
	{
		/* recvMessage already tattled */
		goto fail;
	}

	/*
	 * Check to make sure the message is of the right type.
	 * The type label (pointer) in the message should be
	 * the type descriptor for READER_EVENT_NOTIFICATION.
	 */
	if(&CREADER_EVENT_NOTIFICATION::s_typeDescriptor != pMessage->m_pType)
	{
		goto fail;
	}

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pNtf = (CREADER_EVENT_NOTIFICATION *) pMessage;
	pNtfData = pNtf->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	/*
	 * The ConnectionAttemptEvent parameter must be present.
	 */
	pEvent = pNtfData->getConnectionAttemptEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	/*
	 * The status in the ConnectionAttemptEvent parameter
	 * must indicate connection success.
	 */
	if(ConnectionAttemptStatusType_Success != pEvent->getStatus())
	{
		goto fail;
	}

	/*
	 * Done with the message
	 */
	delete pMessage;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R1_Step5 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: checkConnectionStatus failed\n");
	delete pMessage;
	return -1;
}

inline int TCR_R1::TCR_R1_Step6(void) {
	LLRPTestClient *tempTestClient = new LLRPTestClient(_pReaderHostName);
	tempTestClient->openConnectionToReader();
	delete tempTestClient;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R1_Step6 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R1::TCR_R1_Step7(void) {
	CCLOSE_CONNECTION *        pCmd;
	CMessage *                  pRspMsg;
	CCLOSE_CONNECTION_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CCLOSE_CONNECTION();
	// If the message ID of close connection is 979797, then reader shuts down.
	pCmd->setMessageID(979797);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = _pTestClient->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CCLOSE_CONNECTION_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "getLLRPConfigurationStateValue"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/* Wait for R1 (Default = 10) seconds */
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R1));

	/* Resend CLOSE_CONNECTION to verify the connection is closed */
	pCmd = new CCLOSE_CONNECTION();
	pCmd->setMessageID(121);
	pRspMsg = _pTestClient->transact(pCmd);
	delete pCmd;
	if(NULL != pRspMsg)
	{
		/* connection is not closed, so fail */
		return -1;
	}

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: Shutdown operation is finished\n");
	}

	/*
	 * Victory.
	 */


	CMessage *                  pMessage;
	CREADER_EVENT_NOTIFICATION *pNtf;
	CReaderEventNotificationData *pNtfData;
	CConnectionCloseEvent *   pEvent;

	/*
	 * Expect the notification within 10 seconds.
	 * It is suppose to be the very first message sent.
	 */
	pMessage = _pTestClient->recvMessage(10000);

	/*
	 * recvMessage() returns NULL if something went wrong.
	 */
	if(NULL == pMessage)
	{
		/* recvMessage already tattled */
		goto fail;
	}

	/*
	 * Check to make sure the message is of the right type.
	 * The type label (pointer) in the message should be
	 * the type descriptor for READER_EVENT_NOTIFICATION.
	 */
	if(&CREADER_EVENT_NOTIFICATION::s_typeDescriptor != pMessage->m_pType)
	{
		goto fail;
	}

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pNtf = (CREADER_EVENT_NOTIFICATION *) pMessage;
	pNtfData = pNtf->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	/*
	 * The ConnectionAttemptEvent parameter must be present.
	 */
	pEvent = pNtfData->getConnectionCloseEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	/*
	 * Done with the message
	 */
	delete pMessage;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R1_Step7 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: connectionCloseEvent is not reveived, failed\n");
	delete pMessage;
	return -1;
}

#endif /* TCR_R1_H_ */
