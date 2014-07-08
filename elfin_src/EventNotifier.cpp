/**
 * @file    EventNotifier.cpp
 * @brief
 */

#include <exception>

#include "ROAdmin.h"
#include "EventNotifier.h"

ELFIN::EventNotifier::EventNotifier(LLRPCore* __pLLRPCore)
: _pLLRPCore(__pLLRPCore), _pConfig(__pLLRPCore->m_LLRPConfig) {

}

ELFIN::EventNotifier::~EventNotifier() {

}

int ELFIN::EventNotifier::sendGPIEvent(LLRP::llrp_u32_t GPIPortNumber, LLRP::llrp_u32_t GPIEvent) {
	throw "not implemented";
}

int ELFIN::EventNotifier::sendROSpecEvent(LLRP::llrp_u32_t ROSpecID,
		enum LLRP::EROSpecEventType EventType, LLRP::llrp_u32_t PreemtingROSpecID) {
	int result = 0;
	if (_pConfig->_pEventNotiArray[LLRP::NotificationEventType_ROSpec_Event] == 1) {
		LLRP::CREADER_EVENT_NOTIFICATION *pNoti = new LLRP::CREADER_EVENT_NOTIFICATION();
		LLRP::CReaderEventNotificationData *pData = new LLRP::CReaderEventNotificationData();
		LLRP::CROSpecEvent *pEvent = new LLRP::CROSpecEvent();
		pEvent->setROSpecID(ROSpecID);
		pEvent->setEventType(EventType);
		pEvent->setPreemptingROSpecID(PreemtingROSpecID);
		pData->setROSpecEvent(pEvent);
		LLRP::CUTCTimestamp *pTmStamp;
		pTmStamp = ELFIN::Utils::getCurrentUTCTimestamp();
		pData->setTimestamp(pTmStamp);
		pNoti->setReaderEventNotificationData(pData);

		result = _pLLRPCore->sendMessage((LLRP::CMessage *)pNoti);
		delete pNoti;
	}
	return result;
}

int ELFIN::EventNotifier::sendConnectionAttemptEvent(
		enum LLRP::EConnectionAttemptStatusType EventType) {
	throw "this is done by CConnectionFnCMgr";
}

int ELFIN::EventNotifier::sednConnectionCloseEvent() {
	throw "this is done by CConnectionFnCMgr";
}

int ELFIN::EventNotifier::sendHoppingEvent(int HopTableID,
		int NextChannelIndex) {
	throw "not implemented";
}

int ELFIN::EventNotifier::sendReportBufferLevelWarningEvent(
		int ReportBufferPercentageFull) {
	throw "not implemented";
}

int ELFIN::EventNotifier::sendReportBufferOverflowErrorEvent() {
	throw "not implemented";
}

int ELFIN::EventNotifier::sendReaderExceptionEvent() {
	throw "not implemented";
}

int ELFIN::EventNotifier::sendRFSurveyEvent(int ROSpecID, int SpecIndex,
		enum LLRP::ERFSurveyEventType EventType) {
	throw "not implemented";
}

int ELFIN::EventNotifier::sendAISpecEvent(LLRP::llrp_u32_t ROSpecID,
		LLRP::llrp_u16_t SpecIndex, enum LLRP::EAISpecEventType EventType) {
	int result = 0;
	if (_pConfig->_pEventNotiArray[LLRP::NotificationEventType_AISpec_Event] == 1) {
		LLRP::CREADER_EVENT_NOTIFICATION *pNoti = new LLRP::CREADER_EVENT_NOTIFICATION();
		LLRP::CReaderEventNotificationData *pData = new LLRP::CReaderEventNotificationData();
		LLRP::CAISpecEvent *pEvent = new LLRP::CAISpecEvent();
		pEvent->setROSpecID(ROSpecID);
		pEvent->setSpecIndex(SpecIndex);
		pEvent->setEventType(EventType);
		pData->setAISpecEvent(pEvent);
		LLRP::CUTCTimestamp *pTmStamp;
		pTmStamp = ELFIN::Utils::getCurrentUTCTimestamp();
		pData->setTimestamp(pTmStamp);
		pNoti->setReaderEventNotificationData(pData);

		result = _pLLRPCore->sendMessage((LLRP::CMessage *)pNoti);
		delete pNoti;
	}
	return result;
}

int ELFIN::EventNotifier::sendAntennaEvent(int AntennaID,
		enum LLRP::EAntennaEventType EventType) {
	throw "not implemented";
}

int ELFIN::EventNotifier::sendSpecLoopEvent(LLRP::llrp_u32_t ROSpecID, LLRP::llrp_u32_t LoopCount) {
	int result = 0;
	if (_pConfig->_pEventNotiArray[LLRP::NotificationEventType_SpecLoop_Event] == 1) {
		LLRP::CREADER_EVENT_NOTIFICATION *pNoti = new LLRP::CREADER_EVENT_NOTIFICATION();
		LLRP::CReaderEventNotificationData *pData = new LLRP::CReaderEventNotificationData();
		LLRP::CSpecLoopEvent *pEvent = new LLRP::CSpecLoopEvent();
		pEvent->setROSpecID(ROSpecID);
		if (LoopCount > 4294967295)
			pEvent->setLoopCount(4294967295);
		else
			pEvent->setLoopCount(LoopCount);
		pData->setSpecLoopEvent(pEvent);
		LLRP::CUTCTimestamp *pTmStamp;
		pTmStamp = ELFIN::Utils::getCurrentUTCTimestamp();
		pData->setTimestamp(pTmStamp);
		pNoti->setReaderEventNotificationData(pData);

		result = _pLLRPCore->sendMessage((LLRP::CMessage *)pNoti);
		delete pNoti;
	}
	return result;
}
