/**
 * @file    LLRPReader_Configurations.cpp
 * @brief
 */

#include <time.h>
#include <fstream>
#include "ELFIN_Platform.h"
#include "LLRPReader_Configurations.h"


ELFIN::LLRPReaderConfig::LLRPReaderConfig(const char *__pConfigFileName)
: _pConfigFileName(NULL), _pDefaultROReportSpec(NULL), _pDefaultAccessReportSpec(NULL),
 _pDefaultKeepaliveSpec(NULL), _pDefaultEventsAndReports(NULL),
 _pDefaultReaderEventNotificationSpec(NULL), _pLLRPConfigurationStateValue(0),
 _pDefaultConfig(NULL) {
	this->_pConfigFileName = new char[strlen(__pConfigFileName) + 1];
	strcpy(this->_pConfigFileName, __pConfigFileName);
	this->init();
}

int ELFIN::LLRPReaderConfig::clear() {
	if (_pDefaultROReportSpec != NULL) {
		delete _pDefaultROReportSpec;
	}
	_pDefaultROReportSpec = NULL;
	if (_pDefaultAccessReportSpec != NULL) {
		delete _pDefaultAccessReportSpec;
	}
	_pDefaultAccessReportSpec = NULL;
	if (_pDefaultKeepaliveSpec != NULL) {
		delete _pDefaultKeepaliveSpec;
	}
	_pDefaultKeepaliveSpec = NULL;
	if (_pDefaultEventsAndReports != NULL) {
		delete _pDefaultEventsAndReports;
	}
	_pDefaultEventsAndReports = NULL;
	if (_pDefaultReaderEventNotificationSpec != NULL) {
		delete _pDefaultReaderEventNotificationSpec;
	}
	_pDefaultReaderEventNotificationSpec = NULL;
	_pLLRPConfigurationStateValue = 0;

	if (_pConfigFileName != NULL) {
		delete[] _pConfigFileName;
	}
	_pConfigFileName = NULL;

	if (_pDefaultConfig != NULL) {
		delete _pDefaultConfig;
	}
	_pDefaultConfig = NULL;

	return 0;
}


ELFIN::LLRPReaderConfig::~LLRPReaderConfig() {
	this->clear();
}


int ELFIN::LLRPReaderConfig::init() {
	this->setDefaultSpecs();

	this->loadDefaultConfigFile();

	buildConfigMaps();
	_pLLRPConfigurationStateValue = time(NULL);
	return 0;
}


int ELFIN::LLRPReaderConfig::loadDefaultConfigFile() {
	READER_LOG (LOGTYPE_INFO, "Loading default configuration file\n");
	std::string config_file_name(this->_pConfigFileName);
	std::ifstream conf_ifs(config_file_name.c_str());
	if (!conf_ifs)
	{
		READER_LOG (LOGTYPE_ERROR, "Cannot open config file: %s. Hard-coded default configurations are loaded\n", config_file_name.c_str());
		return -1;
	}
	std::string conf_str((std::istreambuf_iterator<char>(conf_ifs)), std::istreambuf_iterator<char>());
	char *pBuffer = new char[conf_str.size() + 1];
	strcpy( pBuffer , conf_str.c_str() );
	unsigned int nBuffer = conf_str.size();

	/*
	 * Frame appears complete. Time to try to decode it.
	 */
	LLRP::CMessage *          pMessage;
	LLRP::CTypeRegistry *     pTypeRegistry;

	/*
	 * Construct a new frame decoder. It needs the registry
	 * to facilitate decoding.
	 */
	pTypeRegistry = LLRP::getTheTypeRegistry();

	LLRP::CXMLTextDecoder *pXMLDecoder =
			new LLRP::CXMLTextDecoder(pTypeRegistry, pBuffer, nBuffer);

	/*
	 * Make sure we really got one. If not, weird problem.
	 */
	if(pXMLDecoder == NULL)
	{
		return -1;
	}

	/*
	 * Now ask the nice, brand new decoder to decode the frame.
	 * It returns NULL for some kind of error.
	 */
	pMessage = pXMLDecoder->decodeMessage();

	/*
	 * Bye bye and thank you li'l decoder.
	 */

	delete pTypeRegistry;
	//delete pBuffer; // This is freed while deleting XMLDecoder
	delete pXMLDecoder;
	/*
	 * If NULL there was an error. Clean up the
	 * receive state. Return the error.
	 */
	if(NULL == pMessage)
	{
		READER_LOG (LOGTYPE_ERROR, "Failed to decode configuration llrp file: %s\n", pXMLDecoder->m_ErrorDetails.m_pWhatStr);
		return -1;
	}
	this->_pDefaultConfig = pMessage;
	return 0;
}

int ELFIN::LLRPReaderConfig::resetToFactoryDefault() {
	this->clear();
	this->init();
	return 0;
}

int ELFIN::LLRPReaderConfig::setDefaultSpecs() {
	_pDefaultROReportSpec = new LLRP::CROReportSpec();
	_pDefaultROReportSpec->setROReportTrigger(LLRP::ROReportTriggerType_None);
	_pDefaultROReportSpec->setN(0);
	LLRP::CTagReportContentSelector *pSelector = new LLRP::CTagReportContentSelector();
	pSelector->setEnableAccessSpecID(0);
	pSelector->setEnableAntennaID(1);
	pSelector->setEnableChannelIndex(0);
	pSelector->setEnableFirstSeenTimestamp(1);
	pSelector->setEnableInventoryParameterSpecID(0);
	pSelector->setEnableLastSeenTimestamp(1);
	pSelector->setEnablePeakRSSI(0);
	pSelector->setEnableROSpecID(1);
	pSelector->setEnableSpecIndex(0);
	pSelector->setEnableTagSeenCount(0);
	LLRP::CC1G2EPCMemorySelector *pC1G2Selector = new LLRP::CC1G2EPCMemorySelector();
	pC1G2Selector->setEnableCRC(1);
	pC1G2Selector->setEnablePCBits(1);
	pSelector->addAirProtocolEPCMemorySelector((LLRP::CParameter *) pC1G2Selector);
	_pDefaultROReportSpec->setTagReportContentSelector(pSelector);

	_pDefaultAccessReportSpec = new LLRP::CAccessReportSpec();
	_pDefaultAccessReportSpec->setAccessReportTrigger(LLRP::AccessReportTriggerType_Whenever_ROReport_Is_Generated);

	/* Keepalive is disabled as default */
	/* This is just spec definition. Changing this does not start Keepalive */
	_pDefaultKeepaliveSpec = new LLRP::CKeepaliveSpec();
	_pDefaultKeepaliveSpec->setKeepaliveTriggerType(LLRP::KeepaliveTriggerType_Null);
	_pDefaultKeepaliveSpec->setPeriodicTriggerValue(0);

	_pDefaultReaderEventNotificationSpec = new LLRP::CReaderEventNotificationSpec();
	LLRP::CEventNotificationState *pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_AISpec_Event);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_AISpec_Event_With_Details);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Antenna_Event);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_GPI_Event);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_RFSurvey_Event);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_ROSpec_Event);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Reader_Exception_Event);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Report_Buffer_Fill_Warning);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_Upon_Hopping_To_Next_Channel);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);
	pENState = new LLRP::CEventNotificationState();
	pENState->setEventType(LLRP::NotificationEventType_SpecLoop_Event);
	pENState->setNotificationState(0);
	_pDefaultReaderEventNotificationSpec->addEventNotificationState(pENState);

	_pDefaultEventsAndReports = new LLRP::CEventsAndReports();
	_pDefaultEventsAndReports->setHoldEventsAndReportsUponReconnect(FALSE);

	return 0;
}

int ELFIN::LLRPReaderConfig::buildConfigMaps() {
	for (int i = 0;i < EVENT_NOTI_KIND_COUNT;i++) {
		_pEventNotiArray[i] = 0;
	}
	for (std::list<LLRP::CEventNotificationState *>::iterator iter = _pDefaultReaderEventNotificationSpec->beginEventNotificationState();
			iter != _pDefaultReaderEventNotificationSpec->endEventNotificationState();
			iter++) {
		LLRP::CEventNotificationState *pState = (*iter);
		if (pState->getEventType() >= EVENT_NOTI_KIND_COUNT) {
			throw "error";
		}
		_pEventNotiArray[pState->getEventType()]= pState->getNotificationState();
	}
	return TRUE;
}

void ELFIN::LLRPReaderConfig::invalidateStateValue() {
	_pLLRPConfigurationStateValue += time(NULL);
}
