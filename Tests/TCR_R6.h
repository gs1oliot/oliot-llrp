/*
 * TCR_R6.h
 *
 *  Created on: Nov 5, 2013
 *      Author: iot-team
 */

#ifndef TCR_R6_H_
#define TCR_R6_H_


#include <boost/thread.hpp>
#include "LLRPTestClient.h"

#define CONST_R1 10000
#define CONST_R6_11 10000

/* Requirement Purpose: This Test Case Requirement confirms that the Reader correctly performs read
 * operations and provides requested reports.
 */
class TCR_R6 {
public:

	TCR_R6 (const char *__pReaderHostName, LLRPTestClient* __pTestClient) :
		_pReaderHostName(__pReaderHostName), _pTestClient(__pTestClient),
		m_LLRPConfigurationStateValue(0), _pStartTime(0), sentROSpec(NULL)
{};
	const char *_pReaderHostName;
	LLRPTestClient* _pTestClient;
	llrp_u32_t m_LLRPConfigurationStateValue;
	uint64_t _pStartTime;
	CROSpec *sentROSpec;

	int TCR_R6_Prepare1 (void);
	int TCR_R6_Step1 (void);
	int TCR_R6_Step2 (void);
	int TCR_R6_Step3 (void);
	int TCR_R6_Step4 (void);
	int TCR_R6_Step5 (void);
	int TCR_R6_Step6 (void);
	int TCR_R6_Step7 (void);
	int TCR_R6_Step8 (void);
	int TCR_R6_Step9 (void);
	int TCR_R6_Step10 (void);
	int TCR_R6_Step11 (void);
	int TCR_R6_Step12 (void);
	int TCR_R6_Step13 (void);
	int TCR_R6_Step14 (void);
	int TCR_R6_Step15 (void);
	int TCR_R6_Step16 (void);
};

/* Pre-test conditions:
 * Step
 * - An established TCP connection between Reader IUT and Client test software.
 * - One or more UHF Gen2 tags in the field-of-view of the Reader.
 * - No ROSpecs or AccessSpecs are defined in the Reader.
 */
inline int TCR_R6::TCR_R6_Prepare1 (void) {
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
		printf("INFO: TCR_R6_Prepare1 passed\n");
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

/*
 * Step description: Send SET_READER_CONFIG where the
 * default ROReportSpec and AccessReportSpec
 * are set to report all data values at the end of the
 * ROSpec with N=0. Set the
 * ReaderEventNotificationSpec to enable ROSpec
 * and AISpec event notification (EventType=2 and
 * EventType=6) and disable all other event
 * notifications.
 *
 * Expected results: Confirm that successful
 * SET_READER_CONFIG_RESPONSE message is
 * received. Verify that the message and its parameters are
 * correctly encoded.
 *
 */
inline int TCR_R6::TCR_R6_Step1(void) {
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

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R6_Step1"))
	{
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step1 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}


	return 0;
}

/* Step description: Send GET_READER_CONFIG where
 * RequestedData=4.
 *
 * Expected result: Confirm that a successful
 * GET_READER_CONFIG_RESPONSE message is
 * received. Verify that default ROReportSpec matches the
 * ROReportSpec set in step #1.
 */
inline int TCR_R6::TCR_R6_Step2(void) {
	CGET_READER_CONFIG *pCmd;
	CROReportSpec *pRORptSpec;
	CMessage *pRspMsg;
	CGET_READER_CONFIG_RESPONSE *pRsp;

	pCmd = new CGET_READER_CONFIG();
	pCmd->setMessageID(121);
	pCmd->setRequestedData((enum EGetReaderConfigRequestedData) 4);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_READER_CONFIG_RESPONSE *) pRspMsg;

	if (0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R6_Step2")) {
		delete pRspMsg;
		return -1;
	}

	pRORptSpec = pRsp->getROReportSpec();

	if (FALSE
			|| pRORptSpec->getN() != 0
			|| pRORptSpec->getROReportTrigger() != LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec
			|| pRORptSpec->getTagReportContentSelector()->getEnableAccessSpecID() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableAntennaID() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableChannelIndex() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableFirstSeenTimestamp() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableInventoryParameterSpecID() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableLastSeenTimestamp() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnablePeakRSSI() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableROSpecID() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableSpecIndex() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableTagSeenCount() != 1
			|| pRORptSpec->getTagReportContentSelector()->countAirProtocolEPCMemorySelector() == 0
			|| ((CC1G2EPCMemorySelector *) (*pRORptSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector()))->getEnableCRC() != 1
			|| ((CC1G2EPCMemorySelector *) (*pRORptSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector()))->getEnablePCBits() != 1
	) {
		delete pRsp;
		return -1;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step2 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

/* Step description: Send GET_READER_CONFIG where
 * RequestedData=6.
 *
 * Expected result: Confirm that a successful
 * GET_READER_CONFIG_RESPONSE message is
 * received. Verify that default AccessReportSpec matches
 * the AccessReportSpec set in step #1.
 */
inline int TCR_R6::TCR_R6_Step3(void) {
	CGET_READER_CONFIG *pCmd;
	CAccessReportSpec *pAccessRptSpec;
	CMessage *pRspMsg;
	CGET_READER_CONFIG_RESPONSE *pRsp;

	pCmd = new CGET_READER_CONFIG();
	pCmd->setMessageID(121);
	pCmd->setRequestedData((enum EGetReaderConfigRequestedData) 6);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_READER_CONFIG_RESPONSE *) pRspMsg;

	if (0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R6_Step3")) {
		delete pRspMsg;
		return -1;
	}

	pAccessRptSpec = pRsp->getAccessReportSpec();

	if (FALSE
			|| pAccessRptSpec->getAccessReportTrigger() != LLRP::AccessReportTriggerType_Whenever_ROReport_Is_Generated
	) {
		delete pRsp;
		return -1;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step3 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

/* Step description: Send GET_READER_CONFIG where
 * RequestedData=5.
 *
 * Expected result: Confirm that a successful
 * GET_READER_CONFIG_RESPONSE message is
 * received. Verify that default
 * ReaderEventNotificationSpec matches the
 * ReaderEventNotificationSpec set in step #1.
 */
inline int TCR_R6::TCR_R6_Step4(void) {
	CGET_READER_CONFIG *pCmd;
	CReaderEventNotificationSpec *pNotiSpec;
	CMessage *pRspMsg;
	CGET_READER_CONFIG_RESPONSE *pRsp;

	pCmd = new CGET_READER_CONFIG();
	pCmd->setMessageID(121);
	pCmd->setRequestedData((enum EGetReaderConfigRequestedData) 5);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_READER_CONFIG_RESPONSE *) pRspMsg;

	if (0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R6_Step4")) {
		delete pRspMsg;
		return -1;
	}

	pNotiSpec = pRsp->getReaderEventNotificationSpec();
	if (pNotiSpec == NULL) {
		delete pRsp;
		return -1;
	}

	for (std::list<LLRP::CEventNotificationState *>::iterator def_iter = pNotiSpec->beginEventNotificationState();
			def_iter != pNotiSpec->endEventNotificationState();def_iter++) {
		if ((*def_iter)->getEventType() == LLRP::NotificationEventType_AISpec_Event) {
			if ((*def_iter)->getNotificationState() != 1) {
				delete pRsp;
				return -1;
			}
		}
		else if ((*def_iter)->getEventType() == LLRP::NotificationEventType_ROSpec_Event) {
			if ((*def_iter)->getNotificationState() != 1) {
				delete pRsp;
				return -1;
			}
		}
		else {
			if ((*def_iter)->getNotificationState() != 0) {
				delete pRsp;
				return -1;
			}
		}
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step4 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

/* Step description: Send GET_READER_CONFIG where
 * RequestedData=7.
 *
 * Expected result: Confirm that a successful
 * GET_READER_CONFIG_RESPONSE message is
 * received. Record the LLRPConfigurationStateValue
 * reported.
 */
inline int TCR_R6::TCR_R6_Step5(void) {
	if (_pTestClient->checkLLRPConfigurationStateValue(&this->m_LLRPConfigurationStateValue, "TCR_R6_Step5") != 0)
		return -1;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step5 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

/* Step description: Send ADD_ROSPEC with a basic AISpec and
 * null triggers.
 *
 * Expected result: Confirm that a successful ADD_ROSPEC_RESPONSE
 * message is received.
 */
inline int TCR_R6::TCR_R6_Step6(void) {
	CADD_ROSPEC *               pCmd;
	CROSpec *					pBasicROSpec;
	CAISpec *					pBasicAISpec;
	CMessage *                  pRspMsg;
	CADD_ROSPEC_RESPONSE *      pRsp;

	pBasicROSpec = _pTestClient->getBasicROSpec();
	pBasicAISpec = _pTestClient->getBasicAISpec();
	pBasicROSpec->addSpecParameter(pBasicAISpec);
	pCmd = new CADD_ROSPEC();
	pCmd->setMessageID(201);
	pCmd->setROSpec(pBasicROSpec);

	pRspMsg = _pTestClient->transact(pCmd);

	// store the sent rospec. we would use it to compare with the response of GET_ROSPECS
	sentROSpec = pBasicROSpec;
	pCmd->removeSubParameterFromAllList((CParameter *) pBasicROSpec);
	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CADD_ROSPEC_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R6_Step6"))
	{
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step6 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

/* Step description: Send GET_READER_CONFIG where
 * RequestedData=7.
 *
 * Expected result: Confirm that successful
 * GET_READER_CONFIG_RESPONSE message is
 * received. Record the LLRPConfigurationStateValue
 * reported. Verify that this value does not match the state
 * value recorded in step #5.
 */
inline int TCR_R6::TCR_R6_Step7(void) {
	if (_pTestClient->checkLLRPConfigurationStateValue(&this->m_LLRPConfigurationStateValue, "TCR_R6_Step7") != 0)
		return -1;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step7 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

/* Step description: Send ENABLE_ROSPEC where ROSpecID is
 * that sent with ADD_ROSPEC in step #6.
 *
 * Expected result: Confirm that successful
 * GET_READER_CONFIG_RESPONSE message is
 * received. Record the LLRPConfigurationStateValue
 * reported. Verify that this value does not match the state
 * value recorded in step #5.
 */
inline int TCR_R6::TCR_R6_Step8(void) {
	if (_pTestClient->enableROSpec(123, "TCR_R6_Step8") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step8 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

/* Step description: Send GET_ROSPECS.
 *
 * Expected result: Confirm that successful GET_ROSPECS_RESPONSE
 * message is received. Verify that the ROSpec matches
 * the ROSpec set in step #6 except that the ROSpec state
 * is enabled.
 */
inline int TCR_R6::TCR_R6_Step9(void) {
	CGET_ROSPECS *       		pCmd;
	CMessage *                  pRspMsg;
	CGET_ROSPECS_RESPONSE 		*pRsp;

	pCmd = new CGET_ROSPECS();
	pCmd->setMessageID(121);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_ROSPECS_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R6_Step8"))
	{
		delete pRspMsg;
		return -1;
	}

	/* TODO: compare the ROSpec with the already sent one. */
	if (pRsp->countROSpec() != 1) {
		printf ("There are %d ROSpecs in the GET_ROSPECS_RESPONSE\n", pRsp->countROSpec());
		return -1;
	}
	CROSpec *pROSpec = *(pRsp->beginROSpec());

	if (pROSpec->getCurrentState() != LLRP::ROSpecState_Inactive) {
		printf ("ROSpec state is not inactive state. Its %d\n", pROSpec->getCurrentState());
		return -1;
	}

	// Set state to disabled, to comapre with the already sent message.
	pROSpec->setCurrentState(LLRP::ROSpecState_Disabled);
	if (_pTestClient->compareCElems((CElement *) this->sentROSpec, (CElement *) pROSpec) != 0) {
		printf ("ROSpec in GET_ROSPECS_RESPONSE is different from the sent one\n");
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step9 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

/* Step description: Send START_ROSPEC where ROSpecID is
 * that sent with ADD_ROSPEC in step #6.
 *
 * Expected result: Confirm that successful START_ROSPEC_RESPONSE
 * message is received. Record the start time. Confirm that
 * READER_EVENT_NOTIFICATION message for
 * ROSpec start is received.
 */
inline int TCR_R6::TCR_R6_Step10(void) {
	CREADER_EVENT_NOTIFICATION *pRsp;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pEvent;
	// Start ROSpec
	_pStartTime = _pTestClient->getCurrentTimeMilliseconds();
	if (_pTestClient->startROSpec(123, "TCR_R6_Step10") != 0)
		return -1;

	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R6_Step10");
	if (pRsp == NULL)
	{
		goto fail;
	}
	pNtfData = pRsp->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	pEvent = pNtfData->getROSpecEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	if(LLRP::ROSpecEventType_Start_Of_ROSpec != pEvent->getEventType()
			|| 123 != pEvent->getROSpecID())
	{
		goto fail;
	}

	delete pRsp;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step10 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;

	fail:
	printf("ERROR: TCR_R6_Step10 failed\n");
	delete pRsp;
	return -1;
}

/* Step description: Wait for R6.11 (default=10) seconds.
 *
 * Expected result: This wait time is arbitrary so long as the Reader has
 * enough time to complete at least one attempt to read
 * tags.
 */
inline int TCR_R6::TCR_R6_Step11(void) {
	printf ("Waiting for R6.11(default=10) seconds.\n");
	boost::this_thread::sleep_for(boost::chrono::milliseconds(CONST_R6_11));
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step11 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

/* Step description: Send STOP_ROSPEC where ROSpecID is that
 * sent with ADD_ROSPEC in step #6.
 *
 * Expected result: Confirm that successful STOP_ROSPEC_RESPONSE
 * message is received. Verify that
 * READER_EVENT_NOTIFICATION messages for
 * AISpec end and ROSpec end events are received in this
 * respective order. Verify that RO_ACCESS_REPORT
 * message is received after the AISpec end event report
 * and before that ROSpec end event report. Verify that
 * these reports are correctly encoded and that the EPC of
 * the tag in the FOV is present.
 */
inline int TCR_R6::TCR_R6_Step12(void) {
	CREADER_EVENT_NOTIFICATION *pRsp;
	CReaderEventNotificationData *pNtfData;
	CROSpecEvent *				pROEvent;
	CAISpecEvent *				pAIEvent;
	CRO_ACCESS_REPORT *			pReport;

	// Stop ROSpec
	if (_pTestClient->stopROSpec(123, "TCR_R6_Step12") != 0) {
		return -1;
	}

	// Check AISpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R6_Step12");
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
	pReport = _pTestClient->getROAccessReport(10000, "TCR_R6_Step12");
	if (pReport == NULL)
		return -1;
	_pTestClient->printTagReportData(pReport);
	delete pReport;

	// Check ROSpec_Stop event
	// Check Reader event notification
	pRsp = _pTestClient->recvNotification(10000, "TCR_R6_Step12");
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
		printf("INFO: TCR_R6_Step12 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;

	fail:
	printf("ERROR: failed\n");
	if (pRsp != NULL)
		delete pRsp;
	return -1;
}

/* Step description: Send DELETE_ROSPEC where ROSpecID is
 * that sent with ADD_ROSPEC in step #6.
 *
 * Expected result: Confirm that successful
 * DELETE_ROSPEC_RESPONSE message is received.
 * Verify that this message is correctly encoded.
 */
inline int TCR_R6::TCR_R6_Step13(void) {
	// Delete ROSpec
	if (_pTestClient->deleteROSpec(123, "TCR_R6_Step13") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step13 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

/* Step description: Send GET_ROSPECS.
 *
 * Expected result:Confirm that a successful GET_ROSPECS_RESPONSE
 * message is received. Verify that the ROSpec added in
 * step #6 is not present in the ROSpecs reported.
 */
inline int TCR_R6::TCR_R6_Step14(void) {
	CGET_ROSPECS *       		pCmd;
	CMessage *                  pRspMsg;
	CGET_ROSPECS_RESPONSE 		*pRsp;

	pCmd = new CGET_ROSPECS();
	pCmd->setMessageID(121);

	pRspMsg = _pTestClient->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_ROSPECS_RESPONSE *) pRspMsg;

	if(0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R6_Step14"))
	{
		delete pRspMsg;
		return -1;
	}

	if (pRsp->countROSpec() > 0) {
		delete pRspMsg;
		return -1;
	}

	delete pRspMsg;

	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step14 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	return 0;
}

/* Step description: Send GET_READER_CONFIG where
 * RequestedData=7.
 *
 * Expected result: Confirm that successful
 * GET_READER_CONFIG_RESPONSE message is
 * received. Record the LLRPConfigurationStateValue
 * reported. Verify that this value does not match the
 * LLRPConfigurationStateValue recorded in step #7.
 */
inline int TCR_R6::TCR_R6_Step15(void) {
	if (_pTestClient->checkLLRPConfigurationStateValue(&this->m_LLRPConfigurationStateValue, "TCR_R6_Step15") != 0)
		return -1;
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step15 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}
	return 0;
}

/* Step description: Send GET_READER_CONFIG where
 * RequestedData=0, AntennaID=0,
 * GPIPortNum=0, GPOPortNum=0.
 *
 * Expected result: Confirm that successful
 * GET_READER_CONFIG_RESPONSE message is
 * received and contains all mandatory parameters.
 */
inline int TCR_R6::TCR_R6_Step16(void) {
	CGET_READER_CONFIG *pCmd;
	CROReportSpec *pRORptSpec;
	CMessage *pRspMsg;
	CGET_READER_CONFIG_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CGET_READER_CONFIG();
	pCmd->setMessageID(121);
	pCmd->setRequestedData((enum EGetReaderConfigRequestedData) 0);
	pCmd->setAntennaID(0);
	pCmd->setGPIPortNum(0);
	pCmd->setGPOPortNum(0);

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
	if (0 != _pTestClient->checkLLRPStatus(pRsp->getLLRPStatus(), "_")) {
		delete pRspMsg;
		return -1;
	}

	pRORptSpec = pRsp->getROReportSpec();

	if (FALSE
			|| pRORptSpec->getN() != 0
			|| pRORptSpec->getROReportTrigger() != LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec
			|| pRORptSpec->getTagReportContentSelector()->getEnableAccessSpecID() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableAntennaID() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableChannelIndex() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableFirstSeenTimestamp() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableInventoryParameterSpecID() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableLastSeenTimestamp() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnablePeakRSSI() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableROSpecID() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableSpecIndex() != 1
			|| pRORptSpec->getTagReportContentSelector()->getEnableTagSeenCount() != 1
			|| pRORptSpec->getTagReportContentSelector()->countAirProtocolEPCMemorySelector() == 0
			|| ((CC1G2EPCMemorySelector *) (*pRORptSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector()))->getEnableCRC() != 1
			|| ((CC1G2EPCMemorySelector *) (*pRORptSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector()))->getEnablePCBits() != 1
	) {
		delete pRsp;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRsp;

	/*
	 * Tattle progress, maybe
	 */
	if(_pTestClient->m_Verbose)
	{
		printf("INFO: TCR_R6_Step16 passed\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
	}

	/*
	 * Victory.
	 */
	return 0;
}

#endif /* TCR_R6_H_ */
