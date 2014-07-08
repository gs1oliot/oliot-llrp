/*
 * TCR_R10.h
 *
 *  Created on: Nov 5, 2013
 *      Author: iot-team
 */

#ifndef TCR_R10_H_
#define TCR_R10_H_



#include <boost/thread.hpp>
#include "LLRPTestClient.h"

#define CONST_R10_2 5000
#define CONST_R10_4 5000

class TCR_R10 {
public:

	TCR_R10 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient)
{};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;

	int TCR_R10_Prepare (void);
	int TCR_R10_Step1 (void);
	int TCR_R10_Step2 (void);
	int TCR_R10_Step3 (void);
	int TCR_R10_Step4 (void);
};

inline int TCR_R10::TCR_R10_Prepare (void) {
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

inline int TCR_R10::TCR_R10_Step1(void) {
	CSET_READER_CONFIG *pCmd;
	CROReportSpec *pRORptSpec;
	CAccessReportSpec *pAccessRptSpec;
	CReaderEventNotificationSpec *pNotiSpec;
	CMessage *pRspMsg;
	CSET_READER_CONFIG_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CSET_READER_CONFIG();
	pCmd->setMessageID(121);

	pRORptSpec = new CROReportSpec();
	pRORptSpec->setN(0);
	pRORptSpec->setROReportTrigger(LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec);
	LLRP::CTagReportContentSelector *pSelector = new LLRP::CTagReportContentSelector();
	pSelector->setEnableAccessSpecID(1);
	pSelector->setEnableAntennaID(1);
	pSelector->setEnableChannelIndex(1);
	pSelector->setEnableFirstSeenTimestamp(1);
	pSelector->setEnableInventoryParameterSpecID(1);
	pSelector->setEnableLastSeenTimestamp(1);
	pSelector->setEnablePeakRSSI(1);
	pSelector->setEnableROSpecID(1);
	pSelector->setEnableSpecIndex(1);
	pSelector->setEnableTagSeenCount(1);
	LLRP::CC1G2EPCMemorySelector *pC1G2Selector = new LLRP::CC1G2EPCMemorySelector();
	pC1G2Selector->setEnableCRC(1);
	pC1G2Selector->setEnablePCBits(1);
	pSelector->addAirProtocolEPCMemorySelector((LLRP::CParameter *) pC1G2Selector);
	pRORptSpec->setTagReportContentSelector(pSelector);
	pCmd->setROReportSpec(pRORptSpec);

	pAccessRptSpec = new CAccessReportSpec();
	pAccessRptSpec->setAccessReportTrigger(LLRP::AccessReportTriggerType_Whenever_ROReport_Is_Generated);
	pCmd->setAccessReportSpec(pAccessRptSpec);

	pNotiSpec = new LLRP::CReaderEventNotificationSpec();
	LLRP::CEventNotificationState *pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_AISpec_Event);
	pENState->setNotificationState(1);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_AISpec_Event_With_Details);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Antenna_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_GPI_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_RFSurvey_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_ROSpec_Event);
	pENState->setNotificationState(1);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Reader_Exception_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Report_Buffer_Fill_Warning);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Upon_Hopping_To_Next_Channel);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_SpecLoop_Event);
	pENState->setNotificationState(0);
	pNotiSpec->addEventNotificationState(pENState);

	pCmd->setReaderEventNotificationSpec(pNotiSpec);

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
		printf("INFO: SET_READER_CONFIG for ROReportSpec and AccessReportSpec is sent\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}


inline int TCR_R10::TCR_R10_Step2(void) {
	CADD_ROSPEC *               pCmd;
	CROSpec *					pBasicROSpec;
	CAISpec *					pBasicAISpec;
	CMessage *                  pRspMsg;
	CADD_ROSPEC_RESPONSE *      pRsp;

	pBasicROSpec = _pTestClient->getBasicROSpec();
	pBasicAISpec = _pTestClient->getBasicAISpec();
	pBasicROSpec->getROBoundarySpec()->getROSpecStartTrigger()->setROSpecStartTriggerType(LLRP::ROSpecStartTriggerType_Immediate);
	pBasicROSpec->getROBoundarySpec()->getROSpecStopTrigger()->setROSpecStopTriggerType(LLRP::ROSpecStopTriggerType_Duration);
	pBasicROSpec->getROBoundarySpec()->getROSpecStopTrigger()->setDurationTriggerValue(CONST_R10_2);
	pBasicROSpec->addSpecParameter(pBasicAISpec);

	pCmd = new CADD_ROSPEC();
	pCmd->setMessageID(201);
	pCmd->setROSpec(pBasicROSpec);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = _pTestClient->transact(pCmd);

	/*
	 * Done with the command message.
	 * N.B.: And the parameters
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
	 * Cast to a ADD_ROSPEC_RESPONSE message.
	 */
	pRsp = (CADD_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "addROSpec"))
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
		printf("INFO: ROSpec added\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R10::TCR_R10_Step3(void) {
	return _pTestClient->enableROSpec(123, "TCR_R10_Step3");
}

inline int TCR_R10::TCR_R10_Step4(void) {
	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pROEvent;
	CAISpecEvent *				pAIEvent;
	CRO_ACCESS_REPORT *			pReport;

	// Wait R10.4 (default=5) seconds for AISpec and ROSpec to stop.
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R10_4));

	// Check ROSpec_Start event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R10_Step4: Get ROSpecStartEvent");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pROEvent = pNtfData->getROSpecEvent();
	if(NULL == pROEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_Start_Of_ROSpec != pROEvent->getEventType()
			|| 123 != pROEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: ROSpec_Stop event is received\n");
	}

	// Check AISpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R10_Step4: Get AISpecStopEvent");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pAIEvent = pNtfData->getAISpecEvent();
	if(NULL == pAIEvent)
	{
		goto fail;
	}

	if(LLRP::AISpecEventType_End_Of_AISpec != pAIEvent->getEventType()
			|| 123 != pAIEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: AISpec_Stop event is received\n");
	}

	// Check RO_ACCESS_REPORT
	pReport = _pTestClient->getROAccessReport(10000, "TCR_R10_Step4: Get RO_ACCESS_REPORT");
	if (pReport == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport);

	delete pReport;

	// Check ROSpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R10_Step4: Get ROSpecStopEvent");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pROEvent = pNtfData->getROSpecEvent();
	if(NULL == pROEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_End_Of_ROSpec != pROEvent->getEventType()
			|| 123 != pROEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: ROSpec_Stop event is received\n");
	}

	return 0;

	fail:
	printf("ERROR: TCR_R10_Step4 failed\n");
	if (pRsp != NULL)
		delete pRsp;
	return -1;
}

#endif /* TCR_R10_H_ */
