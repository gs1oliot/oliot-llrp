/*
 * TCR_R8.h
 *
 *  Created on: Nov 5, 2013
 *      Author: iot-team
 */

#ifndef TCR_R8_H_
#define TCR_R8_H_



#include <boost/thread.hpp>
#include "LLRPTestClient.h"

#define CONST_R8_2A 10000
#define CONST_R8_2B 10000
#define CONST_R8_6 20000
#define CONST_R8_8A 10000
#define CONST_R8_8B 1000
#define CONST_R8_13 20000
#define CONST_R8_15 20000
#define CONST_R8_17 20000

class TCR_R8 {
public:

	TCR_R8 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient),
		pAccessSpec_Step_9(NULL), m_LLRPConfigurationStateValue(0)
{};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;
	CAccessSpec *	pAccessSpec_Step_9;
	llrp_u32_t m_LLRPConfigurationStateValue;


	int TCR_R8_Prepare (void);
	int TCR_R8_Step1 (void);
	int TCR_R8_Step2 (void);
	int TCR_R8_Step3 (void);
	int TCR_R8_Step4 (void);
	int TCR_R8_Step5 (void);
	int TCR_R8_Step6 (void);
	int TCR_R8_Step6a (void);
	int TCR_R8_Step7 (void);
	int TCR_R8_Step8 (void);
	int TCR_R8_Step9 (void);
	int TCR_R8_Step10 (void);
	int TCR_R8_Step11 (void);
	int TCR_R8_Step12 (void);
	int TCR_R8_Step13 (void);
	int TCR_R8_Step14 (void);
	int TCR_R8_Step15 (void);
	int TCR_R8_Step16 (void);
	int TCR_R8_Step17 (void);
	int TCR_R8_Step18 (void);
	int TCR_R8_Step19 (void);
	int TCR_R8_Step20 (void);
	int TCR_R8_Step21 (void);
	int TCR_R8_Step22 (void);
	int TCR_R8_Step23 (void);
};

inline int TCR_R8::TCR_R8_Prepare (void) {
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
		printf("INFO: TCR_R8_Prepare1 passed\n");
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

inline int TCR_R8::TCR_R8_Step1(void) {
	CSET_READER_CONFIG *pCmd;
	CROReportSpec *pRORptSpec;
	CAccessReportSpec *pAccessRptSpec;
	CReaderEventNotificationSpec *pNotiSpec;
	CMessage *pRspMsg;
	CSET_READER_CONFIG_RESPONSE *pRsp;

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

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CSET_READER_CONFIG_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R8_Step1"))
	{
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R8::TCR_R8_Step2(void) {
	CADD_ROSPEC *               pCmd;
	CROSpec *					pBasicROSpec;
	CAISpec *					pBasicAISpec;
	CMessage *                  pRspMsg;
	CADD_ROSPEC_RESPONSE *      pRsp;

	pBasicROSpec = _pTestClient->getBasicROSpec();
	pBasicROSpec->getROBoundarySpec()->getROSpecStartTrigger()->setROSpecStartTriggerType(LLRP::ROSpecStartTriggerType_Periodic);
	CPeriodicTriggerValue *pPeriodicTriggerValue = new CPeriodicTriggerValue();
	pPeriodicTriggerValue->setOffset(CONST_R8_2A);
	pPeriodicTriggerValue->setPeriod(0); // Period is 0, so this is one-shot inventory.
	pBasicROSpec->getROBoundarySpec()->getROSpecStartTrigger()->setPeriodicTriggerValue(pPeriodicTriggerValue);
	pBasicROSpec->getROBoundarySpec()->getROSpecStopTrigger()->setROSpecStopTriggerType(LLRP::ROSpecStopTriggerType_Duration);
	pBasicROSpec->getROBoundarySpec()->getROSpecStopTrigger()->setDurationTriggerValue(CONST_R8_2B);
	pBasicAISpec = _pTestClient->getBasicAISpec();
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
		printf("INFO: TCR_R8_Step2 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R8::TCR_R8_Step3(void) {
	CADD_ACCESSSPEC *           pCmd;
	CAccessSpec *				pBasicAccessSpec;
	CMessage *                  pRspMsg;
	CADD_ACCESSSPEC_RESPONSE *      pRsp;

	pBasicAccessSpec = _pTestClient->getBasicAccessSpec();
	CC1G2Write *pWrite = _pTestClient->getOpSpecWriteEPC();
	CAccessCommand *pAccessCommand = new CAccessCommand();
	pAccessCommand->addAccessCommandOpSpec((CParameter *) pWrite);
	CC1G2TagSpec *pTagSpec = _pTestClient->getTagSpecMatchAll();
	pAccessCommand->setAirProtocolTagSpec((CParameter *) pTagSpec);
	pBasicAccessSpec->setAccessCommand(pAccessCommand);

	pCmd = new CADD_ACCESSSPEC();
	pCmd->setMessageID(201);
	pCmd->setAccessSpec(pBasicAccessSpec);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CADD_ACCESSSPEC_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R8_Step3"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step3 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R8::TCR_R8_Step4(void) {
	if (_pTestClient->enableAccessSpec(1, "TCR_R8_Step4") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step4 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R8::TCR_R8_Step5(void) {
	if (_pTestClient->enableROSpec(123, "TCR_R8_Step5") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step5 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R8::TCR_R8_Step6(void) {
	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pROEvent;
	CAISpecEvent *				pAIEvent;
	CRO_ACCESS_REPORT *			pReport;
	CTagReportData *			pTagReportData;
	CC1G2WriteOpSpecResult *	pC1G2WriteOpSpecResult;
	int 						ACOpSpecResultFound;

	// Wait R10.4 (default=5) seconds for AISpec and ROSpec to stop.
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R8_6));

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
	if (pReport->countTagReportData() == 0) {
		printf ("There is no tag report data in the RO_ACCESS_REPORT\n");
		return -1;
	}
	// Pick the first report
	ACOpSpecResultFound = 0;
	for (std::list<CTagReportData *>::iterator iter = pReport->beginTagReportData();
			iter != pReport->endTagReportData(); iter++) {
		pTagReportData = (*iter);
		if (pTagReportData->countAccessCommandOpSpecResult() == 0) {
			continue;
		}
		else {
			ACOpSpecResultFound = 1;
			break;
		}
	}
	if (ACOpSpecResultFound == 0) {
		printf ("There is no AccessCommandOpSpecResult in the RO_ACCESS_REPORT\n");
		return -1;
	}
	pC1G2WriteOpSpecResult =
			(CC1G2WriteOpSpecResult *) (*(pTagReportData->beginAccessCommandOpSpecResult()));
	if (pC1G2WriteOpSpecResult->getResult() != LLRP::C1G2WriteResultType_Success) {
		printf ("The C1G2Write operation returned failure\n");
		return -1;
	}

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

inline int TCR_R8::TCR_R8_Step6a(void) {
	CGET_ACCESSSPECS *       		pCmd;
	CMessage *                  pRspMsg;
	CGET_ACCESSSPECS_RESPONSE 		*pRsp;

	pCmd = new CGET_ACCESSSPECS();
	pCmd->setMessageID(121);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_ACCESSSPECS_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R8_Step6a"))
	{
		delete pRspMsg;
		return -1;
	}

	if (pRsp->countAccessSpec() != 0) {
		delete pRspMsg;
		return -1;
	}

	/* TODO: compare the ROSpec with the already sent one. */

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step6a passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R8::TCR_R8_Step7(void) {
	if (_pTestClient->deleteROSpec(123, "TCR_R8_Step7") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step7 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R8::TCR_R8_Step8(void) {
	CADD_ROSPEC *               pCmd;
	CROSpec *					pBasicROSpec;
	CAISpec *					pBasicAISpec;
	CMessage *                  pRspMsg;
	CADD_ROSPEC_RESPONSE *      pRsp;

	pBasicROSpec = _pTestClient->getBasicROSpec();
	pBasicROSpec->getROBoundarySpec()->getROSpecStartTrigger()->setROSpecStartTriggerType(LLRP::ROSpecStartTriggerType_Periodic);
	CPeriodicTriggerValue *pPeriodicTriggerValue = new CPeriodicTriggerValue();
	pPeriodicTriggerValue->setOffset(0);
	pPeriodicTriggerValue->setPeriod(CONST_R8_8A); // Period is 0, so this is one-shot inventory.
	pBasicROSpec->getROBoundarySpec()->getROSpecStartTrigger()->setPeriodicTriggerValue(pPeriodicTriggerValue);
	pBasicROSpec->getROBoundarySpec()->getROSpecStopTrigger()->setROSpecStopTriggerType(LLRP::ROSpecStopTriggerType_Duration);
	pBasicROSpec->getROBoundarySpec()->getROSpecStopTrigger()->setDurationTriggerValue(CONST_R8_8B);
	pBasicAISpec = _pTestClient->getBasicAISpec();
	CInventoryParameterSpec *pInventoryParameterSpec = _pTestClient->getROSpecFilterMatchAll();
	pBasicAISpec->addInventoryParameterSpec(pInventoryParameterSpec);
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
		printf("INFO: TCR_R8_Step8 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

inline int TCR_R8::TCR_R8_Step9(void) {
	CADD_ACCESSSPEC *           pCmd;
	CAccessSpec *				pBasicAccessSpec;
	CMessage *                  pRspMsg;
	CADD_ACCESSSPEC_RESPONSE *      pRsp;

	pBasicAccessSpec = _pTestClient->getBasicAccessSpec();
	pBasicAccessSpec->setROSpecID(123);
	pBasicAccessSpec->getAccessSpecStopTrigger()->setOperationCountValue(0);
	CC1G2Read *pRead = _pTestClient->getOpSpecReadEPC();
	CAccessCommand *pAccessCommand = new CAccessCommand();
	pAccessCommand->addAccessCommandOpSpec((CParameter *) pRead);
	CC1G2TagSpec *pTagSpec = _pTestClient->getTagSpecMatchAll();
	pAccessCommand->setAirProtocolTagSpec((CParameter *) pTagSpec);
	pBasicAccessSpec->setAccessCommand(pAccessCommand);

	pCmd = new CADD_ACCESSSPEC();
	pCmd->setMessageID(201);
	pCmd->setAccessSpec(pBasicAccessSpec);

	pAccessSpec_Step_9 = pCmd->getAccessSpec();
	pCmd->removeSubParameterFromAllList(pCmd->getAccessSpec());

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CADD_ACCESSSPEC_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R8_Step3"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step9 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R8::TCR_R8_Step10(void) {
	if (_pTestClient->enableAccessSpec(1, "TCR_R8_Step10") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step10 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R8::TCR_R8_Step11(void) {
	if (_pTestClient->enableROSpec(123, "TCR_R8_Step11") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step11 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

// This test case is sending START_ROSPEC to the reader.
// But the periodic ROSpec starts automatically and the reply of this message should be error message.
// So, just ignore and let it always return 0.
inline int TCR_R8::TCR_R8_Step12(void) {
#if 1
	return 0;
#else
	_pStartTime = _pTestClient->getCurrentTimeMilliseconds();
	if (_pTestClient->startROSpec(123, "TCR_R7_Step4") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R7_Step4 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
#endif
}

inline int TCR_R8::TCR_R8_Step13(void) {
	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pROEvent;
	CAISpecEvent *				pAIEvent;
	CRO_ACCESS_REPORT *			pReport;
	CTagReportData *pData;
	CC1G2ReadOpSpecResult *pOpResult;
	LLRP::CEPC_96 *				pEPCData;
	LLRP::llrp_u96_t 			pEPCID;

	// Wait R10.4 (default=5) seconds for AISpec and ROSpec to stop.
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R8_13));

	// Check ROSpec_Start event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R8_Step13: Get ROSpecStartEvent");
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
	pRsp = _pTestClient->recvNotification(10000, "TCR_R8_Step13: Get AISpecStopEvent");
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
	pReport = _pTestClient->getROAccessReport(10000, "TCR_R8_Step13: Get RO_ACCESS_REPORT");
	if (pReport == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport);
	for (std::list<CTagReportData *>::iterator iter = pReport->beginTagReportData();
			iter != pReport->endTagReportData(); iter++) {
		pData = (*iter);
		if (pData->countAccessCommandOpSpecResult() != 1) {
			printf ("There is %d ACOpSpecResult in the tag report. This should be only one.\n", pData->countAccessCommandOpSpecResult());
			return -1;
		}
		pOpResult = (CC1G2ReadOpSpecResult *) (*pData->beginAccessCommandOpSpecResult());
		pEPCData = (CEPC_96 *) pData->getEPCParameter();
		pEPCID = pEPCData->getEPC();

		for (int j = 0;j < 6;j++) { // first 16-bits are PC and CRC
			if ((pEPCID.m_aValue[j * 2] != pOpResult->getReadData().m_pValue[j + 2] / 0x100)
					&& (pEPCID.m_aValue[j * 2 + 1] != pOpResult->getReadData().m_pValue[j + 4]) % 0x100) {
				printf ("EPCID of tag report and c1g2read result are different.\n");
				return -1;
			}
		}

	}

	delete pReport;
	printf ("RO_ACCESS_REPORT is correct\n");

	// Check ROSpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R8_Step13: Get ROSpecStopEvent");
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

	// remove remaining messages
	while (1) {
		pRsp = _pTestClient->recvNotification(0, "TCR_R8_Step13: Removing all notifications messages in buffer\n");
		if (pRsp == NULL)
			break;
	}
	while (1) {
		pReport = _pTestClient->getROAccessReport(0, "TCR_R8_Step13: Removing all RO_ACCESS_REPORTS in buffer\n");
		if (pReport == NULL)
			break;
	}
	while (1) {
		pRsp = _pTestClient->recvNotification(10000, "TCR_R8_Step13: Removing all notifications messages in buffer\n");
		if (pRsp->getReaderEventNotificationData()->getROSpecEvent() != NULL
				&& pRsp->getReaderEventNotificationData()->getROSpecEvent()->getEventType() == LLRP::ROSpecEventType_End_Of_ROSpec) {
			break;
		}
	}
	while (1) {
		pReport = _pTestClient->getROAccessReport(5000, "TCR_R8_Step13: Removing all RO_ACCESS_REPORTS in buffer\n");
		if (pReport == NULL)
			break;
	}

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step13 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;


	fail:
	printf("ERROR: TCR_R8_Step13 failed\n");
	if (pRsp != NULL)
		delete pRsp;
	return -1;
}

inline int TCR_R8::TCR_R8_Step14(void) {
	if (_pTestClient->disableAccessSpec(1, "TCR_R8_Step14") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step14 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R8::TCR_R8_Step15(void) {
	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pROEvent;
	CAISpecEvent *				pAIEvent;
	CRO_ACCESS_REPORT *			pReport;
	CTagReportData *pData;

	// Wait R8.15 (default=20) seconds for AISpec and ROSpec to stop.
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R8_15));

	// Check ROSpec_Start event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R8_Step15: Get ROSpecStartEvent");
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
	pRsp = _pTestClient->recvNotification(10000, "TCR_R8_Step15: Get AISpecStopEvent");
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
	pReport = _pTestClient->getROAccessReport(10000, "TCR_R8_Step15: Get RO_ACCESS_REPORT");
	if (pReport == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport);
	for (std::list<CTagReportData *>::iterator iter = pReport->beginTagReportData();
			iter != pReport->endTagReportData(); iter++) {
		pData = (*iter);
		if (pData->countAccessCommandOpSpecResult() != 0) {
			printf ("There is %d ACOpSpecResult in the tag report. This should be 0.\n", pData->countAccessCommandOpSpecResult());
			return -1;
		}
	}

	delete pReport;
	printf ("RO_ACCESS_REPORT is correct\n");

	// Check ROSpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R8_Step15: Get ROSpecStopEvent");
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

	// remove remaining messages
	while (1) {
		pRsp = _pTestClient->recvNotification(0, "TCR_R8_Step15: Removing all notifications messages in buffer\n");
		if (pRsp == NULL)
			break;
	}
	while (1) {
		pReport = _pTestClient->getROAccessReport(0, "TCR_R8_Step15: Removing all RO_ACCESS_REPORTS in buffer\n");
		if (pReport == NULL)
			break;
	}

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step15 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;


	fail:
	printf("ERROR: TCR_R8_Step15 failed\n");
	if (pRsp != NULL)
		delete pRsp;
	return -1;
}

inline int TCR_R8::TCR_R8_Step16(void) {
	if (_pTestClient->disableROSpec(123, "TCR_R8_Step16") != 0)
		return -1;

	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CRO_ACCESS_REPORT *			pReport;
	// remove remaining messages
	while (1) {
		pRsp = _pTestClient->recvNotification(2000, "TCR_R8_Step17: Removing all notifications messages in buffer\n");
		if (pRsp == NULL)
			break;
	}
	while (1) {
		pReport = _pTestClient->getROAccessReport(2000, "TCR_R8_Step17: Removing all RO_ACCESS_REPORTS in buffer\n");
		if (pReport == NULL)
			break;
	}
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step16 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R8::TCR_R8_Step17(void) {
	CREADER_EVENT_NOTIFICATION *pRsp = NULL;
	CRO_ACCESS_REPORT *			pReport;

	// Wait R10.4 (default=5) seconds for AISpec and ROSpec to stop.
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R8_17));

	// Check RO_ACCESS_REPORT
	pReport = _pTestClient->getROAccessReport(0, "TCR_R8_Step17: Get RO_ACCESS_REPORT");
	if (pReport != NULL) {
		printf ("RO_ACCESS_REPORT is received. Wrong result.\n");
		return -1;
	}

	printf ("No RO_ACCESS_REPORT is received. Correct result.\n");

	// remove remaining messages
	while (1) {
		pRsp = _pTestClient->recvNotification(0, "TCR_R8_Step17: Removing all notifications messages in buffer\n");
		if (pRsp == NULL)
			break;
	}
	while (1) {
		pReport = _pTestClient->getROAccessReport(0, "TCR_R8_Step17: Removing all RO_ACCESS_REPORTS in buffer\n");
		if (pReport == NULL)
			break;
	}


	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step17 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R8::TCR_R8_Step18(void) {
	CGET_ACCESSSPECS *       		pCmd;
	CMessage *                  pRspMsg;
	CGET_ACCESSSPECS_RESPONSE 		*pRsp;

	pCmd = new CGET_ACCESSSPECS();
	pCmd->setMessageID(121);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_ACCESSSPECS_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R8_Step18"))
	{
		delete pRspMsg;
		return -1;
	}

	if (pRsp->countAccessSpec() != 1) {
		delete pRspMsg;
		return -1;
	}

	if (_pTestClient->compareCElems((CElement *) (*pRsp->beginAccessSpec()), pAccessSpec_Step_9) != 0) {
		printf ("Current AccessSpec and the one added in Step 9 are different.\n");
		delete pRspMsg;
		return -1;
	}

	/* TODO: compare the ROSpec with the already sent one. */

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step18 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R8::TCR_R8_Step19(void) {
	if (_pTestClient->checkLLRPConfigurationStateValue(&this->m_LLRPConfigurationStateValue, "TCR_R6_Step19") != 0)
		return -1;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step19 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R8::TCR_R8_Step20(void) {
	if (_pTestClient->deleteAccessSpec(1, "TCR_R8_Step20") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step20 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

inline int TCR_R8::TCR_R8_Step21(void) {
	CADD_ACCESSSPEC *           pCmd;
	CAccessSpec *				pBasicAccessSpec;
	CMessage *                  pRspMsg;
	CADD_ACCESSSPEC_RESPONSE *      pRsp;

	pBasicAccessSpec = _pTestClient->getBasicAccessSpec();
	pBasicAccessSpec->setROSpecID(123);
	pBasicAccessSpec->getAccessSpecStopTrigger()->setOperationCountValue(0);
	CC1G2Read *pRead = _pTestClient->getOpSpecReadEPC();
	CAccessCommand *pAccessCommand = new CAccessCommand();
	pAccessCommand->addAccessCommandOpSpec((CParameter *) pRead);
	CC1G2TagSpec *pTagSpec = _pTestClient->getTagSpecMatchAll();
	pAccessCommand->setAirProtocolTagSpec((CParameter *) pTagSpec);
	pBasicAccessSpec->setAccessCommand(pAccessCommand);

	pCmd = new CADD_ACCESSSPEC();
	pCmd->setMessageID(201);
	pCmd->setAccessSpec(pBasicAccessSpec);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CADD_ACCESSSPEC_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R8_Step3"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step3 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R8::TCR_R8_Step22(void) {
	CGET_ACCESSSPECS *       		pCmd;
	CMessage *                  pRspMsg;
	CGET_ACCESSSPECS_RESPONSE 		*pRsp;

	pCmd = new CGET_ACCESSSPECS();
	pCmd->setMessageID(121);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_ACCESSSPECS_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R8_Step22"))
	{
		delete pRspMsg;
		return -1;
	}

	if (pRsp->countAccessSpec() != 1) {
		delete pRspMsg;
		return -1;
	}

	if (_pTestClient->compareCElems((CElement *) (*pRsp->beginAccessSpec()), pAccessSpec_Step_9) != 0) {
		printf ("Current AccessSpec and the one added in Step 9 are different.\n");
		delete pRspMsg;
		return -1;
	}
	printf ("Current AccessSpec and the one added in Step 9 are same.\n");


	/* TODO: compare the ROSpec with the already sent one. */

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R8_Step22 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

inline int TCR_R8::TCR_R8_Step23(void) {
	if (_pTestClient->checkLLRPConfigurationStateValue(&this->m_LLRPConfigurationStateValue, "TCR_R6_Step23") != 0)
		return -1;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step23 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

#endif /* TCR_R8_H_ */
