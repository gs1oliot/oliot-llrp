/*
 * TCR_R3.h
 *
 *  Created on: Nov 5, 2013
 *      Author: iot-team
 */

#ifndef TCR_R3_H_
#define TCR_R3_H_


#include <boost/thread.hpp>
#include "LLRPTestClient.h"

class TCR_R3 {
public:

	TCR_R3 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient)
		{};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;

	int TCR_R3_Prepare (void);
	int TCR_R3_Step1 (void);
};

inline int TCR_R3::TCR_R3_Prepare (void) {
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
		printf("INFO: TCR_R3_Prepare1 passed\n");
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

inline int TCR_R3::TCR_R3_Step1(void) {
	CGET_READER_CAPABILITIES *pCmd;
	CMessage *pRspMsg;
	CGET_READER_CAPABILITIES_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CGET_READER_CAPABILITIES();
	pCmd->setMessageID(121);
	pCmd->setRequestedData((enum EGetReaderCapabilitiesRequestedData) 0);

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
	pRsp = (CGET_READER_CAPABILITIES_RESPONSE *) pRspMsg;

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

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R3_Step1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

#endif /* TCR_R3_H_ */
