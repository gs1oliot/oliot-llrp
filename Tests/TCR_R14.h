/*
 * TCR_R14.h
 *
 *  Created on: Nov 5, 2013
 *      Author: iot-team
 */

#ifndef TCR_R14_H_
#define TCR_R14_H_

#include <sys/time.h>
#include <inttypes.h>
#include <ctime>
#include <boost/thread.hpp>
#include "LLRPTestClient.h"

#define CONST_R1 10000

class TCR_R14 {
public:

	TCR_R14 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient)
{};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;

	int TCR_R14_Prepare (void);
	int TCR_R14_Step1 (void);
	int TCR_R14_Step2 (void);
	int TCR_R14_Step3 (void);
	int TCR_R14_Step4 (void);

	int64_t getCurrentTimeMs();
};

inline int TCR_R14::TCR_R14_Prepare (void) {
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
		printf("INFO: TCR_R14_Prepare1 passed\n");
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

inline int TCR_R14::TCR_R14_Step1(void) {
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
	pKaSpec->setKeepaliveTriggerType((enum LLRP::EKeepaliveTriggerType) 1);
	pKaSpec->setPeriodicTriggerValue(5000);

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
		printf("INFO: TCR_R14_Step1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}


inline int TCR_R14::TCR_R14_Step2(void) {
	CMessage *                  pMessage;
	CKEEPALIVE_ACK *pNtf;

	int64_t before_time = this->getCurrentTimeMs();
	int duration = 0;

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
	if(&CKEEPALIVE::s_typeDescriptor != pMessage->m_pType)
	{
		goto fail;
	}

	/*
	 * Done with the message
	 */
	delete pMessage;

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pNtf = new CKEEPALIVE_ACK();

	pNtf->setMessageID(121);

	_pTestClient->sendMessage(pNtf);

	delete pNtf;

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
	if(&CKEEPALIVE::s_typeDescriptor != pMessage->m_pType)
	{
		goto fail;
	}

	/*
	 * Done with the message
	 */
	delete pMessage;

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pNtf = new CKEEPALIVE_ACK();

	pNtf->setMessageID(121);

	_pTestClient->sendMessage(pNtf);

	delete pNtf;

	duration = (int) (this->getCurrentTimeMs() - before_time);
	if (duration > 5000 * 2 + 1000) {
		goto fail;
	}
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: elapsed time: %d\n", duration);
		printf("INFO: TCR_R14_Step2 passed\n");
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
	printf("ERROR: failed\n");
	delete pMessage;
	return -1;
}

inline int TCR_R14::TCR_R14_Step3(void) {
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
	pKaSpec->setKeepaliveTriggerType((enum LLRP::EKeepaliveTriggerType) 0);
	pKaSpec->setPeriodicTriggerValue(5000);

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
		printf("INFO: TCR_R14_Step3 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R14::TCR_R14_Step4(void) {
	CMessage *                  pMessage;
	CKEEPALIVE_ACK *pNtf;

	/*
	 * Expect the notification within 10 seconds.
	 * It is suppose to be the very first message sent.
	 */
	pMessage = _pTestClient->recvMessage(11000);

	/*
	 * recvMessage() returns NULL if something went wrong.
	 */
	if(NULL == pMessage)
	{
		/* recvMessage already tattled */
		goto success;
	}

	/*
	 * Check to make sure the message is of the right type.
	 * The type label (pointer) in the message should be
	 * the type descriptor for READER_EVENT_NOTIFICATION.
	 */
	if(&CKEEPALIVE::s_typeDescriptor != pMessage->m_pType)
	{
		goto fail;
	}

	/*
	 * Done with the message
	 */
	delete pMessage;

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pNtf = new CKEEPALIVE_ACK();

	pNtf->setMessageID(121);

	_pTestClient->sendMessage(pNtf);

	delete pNtf;


	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R14_Step4 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return -1;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: failed\n");
	delete pMessage;
	return -1;

	success:
	return 0;
}

inline int64_t TCR_R14::getCurrentTimeMs() {
	struct timeval tv;

	gettimeofday(&tv, NULL);

	int64_t ret = tv.tv_usec;
	/* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
	ret /= 1000;

	/* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
	ret += (tv.tv_sec * 1000);

	return ret;
}

#endif /* TCR_R14_H_ */
