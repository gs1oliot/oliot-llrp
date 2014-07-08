/*
 * TCR_R5.h
 *
 *  Created on: Nov 5, 2013
 *      Author: iot-team
 */

#ifndef TCR_R5_H_
#define TCR_R5_H_


#include <boost/thread.hpp>
#include "LLRPTestClient.h"

#define CONST_R1 10000

class TCR_R5 {
public:

	TCR_R5 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient)
{};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;

	int TCR_R5_Prepare (void);
	int TCR_R5_Step1 (void);
	int TCR_R5_Step2 (void);
	int TCR_R5_Step3 (void);
	int TCR_R5_Step4 (void);
	int TCR_R5_Step5 (void);
	int TCR_R5_Step6 (void);
	int TCR_R5_Step7 (void);
	int TCR_R5_Step8 (void);
	int TCR_R5_Step9 (void);
	int TCR_R5_Step10 (void);
	int TCR_R5_Step11 (void);
};

inline int TCR_R5::TCR_R5_Prepare (void) {
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
		printf("INFO: Connection status OK\n");
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

inline int TCR_R5::TCR_R5_Step1(void) {
	CSET_READER_CONFIG *pCmd;
	CKeepaliveSpec *pKaSpec;
	CMessage *pRspMsg;
	CSET_READER_CONFIG_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CSET_READER_CONFIG();
	pCmd->setMessageID(121);

	pKaSpec = new CKeepaliveSpec();
	pKaSpec->setKeepaliveTriggerType((enum LLRP::EKeepaliveTriggerType) 2);
	pKaSpec->setPeriodicTriggerValue(10000);

	pCmd->setKeepaliveSpec(pKaSpec);

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
	pRsp = (CSET_READER_CONFIG_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(-2 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "getLLRPConfigurationStateValue"))
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
		printf("INFO: Got LLRPConfigurationStateValue\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}


inline int TCR_R5::TCR_R5_Step2(void) {
	CGET_READER_CAPABILITIES *pCmd;
	CMessage *pRspMsg;
	CGET_READER_CAPABILITIES_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CGET_READER_CAPABILITIES();
	pCmd->setMessageID(121);
	pCmd->setRequestedData((enum EGetReaderCapabilitiesRequestedData) 5);

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
	if (-2 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "getLLRPConfigurationStateValue")) {
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: Got error from the wrong GET_READER_CAPABILITIES message\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R5::TCR_R5_Step3(void) {
	// Because LTK automatically trims unsupported parameter, omit this step.
	return 0;
}

inline int TCR_R5::TCR_R5_Step4(void) {
	// Because LTK automatically sets default values, omit this step.
	// (cannot remove ResetToFactoryDefault parameter. it is just value and has default value)
	return 0;
}

inline int TCR_R5::TCR_R5_Step5(void) {
	// Because LTK does not support this type of erroneous message, omit this step.
	return 0;
}

inline int TCR_R5::TCR_R5_Step6(void) {
	// Because LTK automatically trims unsupported parameter, omit this step.
	return 0;
}

inline int TCR_R5::TCR_R5_Step7(void) {
	// Because LTK automatically trims unsupported message, omit this step.
	return 0;
}

inline int TCR_R5::TCR_R5_Step8(void) {
	// Because LTK automatically trims unsupported parameter, omit this step.
	return 0;
}

inline int TCR_R5::TCR_R5_Step9(void) {
	// Because this test is about the LLRP 1.0.1 message handling, omit this step.
	return 0;
}

inline int TCR_R5::TCR_R5_Step10(void) {
	CERROR_MESSAGE *pCmd;
	CMessage *pRspMsg;

	/*
	 * Compose the command message
	 */
	pCmd = new CERROR_MESSAGE();
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
	if(NULL != pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	return 0;
}

inline int TCR_R5::TCR_R5_Step11(void) {
	// Because LTK does not support this method, omit this step.
	return 0;
}

#endif /* TCR_R5_H_ */
