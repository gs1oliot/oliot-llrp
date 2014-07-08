/**
 * @file    LLRPCore.cpp
 * @brief
 */

#include <vector>
#include <iterator>
//#include <boost/thread/thread.hpp>

#include "CConnectionFnCMgr.h"
#include "EventNotifier.h"
#include "Stubs/StubReader.h"
#include "AOAdmin.h"
#include "ROAdmin.h"
#include "GPIEventListener.h"
#include "ELFIN_Platform.h"
#include "LLRPCore.h"

ELFIN::LLRPCore::LLRPCore(ProgramOptions* __progOpt)
: m_LLRPConfig(NULL), m_physicalReader(NULL), m_ROAdmin(NULL), m_AOAdmin(NULL),
  m_connMgr(NULL), m_EventNotifier(NULL), m_shutdown(0), m_receivedCloseConnection(0),
  m_justInited(0) {
	m_progOpt = __progOpt;
	setLogLevel(m_progOpt->getLogLevel());
	m_progOpt->printOptions();
	m_ErrorDetails.resultCodeAndWhatStr(LLRP::RC_OK, "");
}

int ELFIN::LLRPCore::init() {
	READER_LOG(LOGTYPE_DEBUG, "Locking for initialization\n");
	this->m_initLock.lock();
	READER_LOG(LOGTYPE_DEBUG, "Locked for initialization\n");
	StubReaderApp *pStubApp = NULL;
	// If this is not the first initialization, save pointer of ReaderApp and reconnect later
	if (m_physicalReader != NULL) {
		m_physicalReader->m_StubApp->_isConnectedToReader = 0;
		pStubApp = m_physicalReader->m_StubApp;
	}
	this->clear();
	m_LLRPConfig = new LLRPReaderConfig(m_progOpt->getLLRPFileName().c_str());
	m_connMgr = new CConnectionFnCMgr();
	m_physicalReader = new StubReader(this);
	m_ROAdmin = new ROAdmin(this);
	m_AOAdmin = new AOAdmin(this);
	m_EventNotifier = new EventNotifier(this);
	m_justInited = 1;

	setLogLevel(m_progOpt->getLogLevel());

	// if the stub reader app is not emulated one, then connect again
	if (pStubApp != NULL && pStubApp->_isEmulator == 0) {
		this->connectReaderApp(pStubApp);
	}

	this->m_initLock.unlock();

	return 0;
}

int ELFIN::LLRPCore::clear() {
	if (m_ROAdmin != NULL) {
		delete m_ROAdmin;
		m_ROAdmin = NULL;
	}
	if (m_AOAdmin != NULL) {
		delete m_AOAdmin;
		m_AOAdmin = NULL;
	}
	if (m_physicalReader != NULL) {
		delete m_physicalReader;
		m_physicalReader = NULL;
	}
	if (m_connMgr != NULL) {
		if (this->m_connMgr->isConnected() == 1)
			m_connMgr->sendConnectionCloseEvent();
		delete m_connMgr;
		m_connMgr = NULL;
	}
	if (m_LLRPConfig != NULL) {
		delete m_LLRPConfig;
		m_LLRPConfig = NULL;
	}
	if (m_EventNotifier != NULL) {
		delete m_EventNotifier;
		m_EventNotifier = NULL;
	}
	return 0;
}
ELFIN::LLRPCore::~LLRPCore() {
	READER_LOG (LOGTYPE_TRACE, "Destroying LLRPCore...\n");
	//boost::mutex::scoped_lock lock(this->_initLock);
	this->m_shutdown = 1;

	this->clear();
}

int ELFIN::LLRPCore::startConnection(void) {
	if (m_connMgr->isConnected() == 0) {
		int connResult = m_connMgr->startConnection(m_progOpt);
		if (m_physicalReader->m_StubApp != NULL && m_physicalReader->getStubAppIsEmulator() == 0)
			m_physicalReader->m_StubApp->notifyConnectionResultCallback(connResult);
		return connResult;
	}
	else {
		return 0;
	}
}

int ELFIN::LLRPCore::run() {
	LLRP::CMessage *pCmd;
	LLRP::CMessage **pRsp;
	for (;;) {
		this->m_initLock.lock();
		// Check whether LLRP Wrapper is connected to FnC server for every loop.
		if (m_connMgr->isConnected() == 0) { // Not connected
			this->m_initLock.unlock();
			if (m_physicalReader->m_StubApp->_isEmulator == 1) {
				// NOT_REACHED
				READER_LOG (LOGTYPE_FATAL, "Unknown error was occurred\n");
			}
			else {
				// If the LLRP Wrapper is just initialized, send callback to StubReaderApp
				if (m_justInited == 1) {
					READER_LOG(LOGTYPE_INFO, "Ready to start connection...\n");
					m_physicalReader->m_StubApp->readyToConnectCallback();
					boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
				}
				// If LLRP Wrapper is not connected during execution, it is closed connection error.
				// Restore LLRPCore to initial state and send callback to StubReaderApp.
				else {
					READER_LOG(LOGTYPE_INFO, "Connection is closed. Retrying connection...\n");
					this->init();
					m_physicalReader->m_StubApp->connectionClosedCallback();
					boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
				}
			}
			if (m_shutdown == 1) {
				boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
				break;
			}
			else {
				continue;
			}
		}
		else { // Connection is alive
			// If this is the first check of connection after initialized, send callback to StubReaderApp
			if (m_justInited == 1) {
				m_physicalReader->m_StubApp->initializaionFinishedCallback();
				m_justInited = 0;
			}
			// Sleep thread for little time, to yield CPU
			boost::this_thread::sleep_for(boost::chrono::milliseconds(20));
			// Immediate poll of message in recv buffer
			pCmd = m_connMgr->recvMessage(0);
			if (pCmd == NULL && m_connMgr->isConnected()) {
				const LLRP::CErrorDetails *pError = m_connMgr->getRecvError();
				switch (pError->m_eResultCode) {
				case LLRP::RC_RecvTimeout:
					// Message is not arrived yet. Just break and continue.
					this->m_initLock.unlock();
					continue;
				case LLRP::RC_RecvEOF:
					/// @fixme  close connection more elegantly. (not using progOpt to get ReaderInitiatedOpt)
					READER_LOG (LOGTYPE_ERROR, "End-of-File is received from the server.\n");
					this->m_initLock.unlock();
					this->m_connMgr->closeConnectionToFnC(this->m_progOpt->getReaderInitiatedOpt());
					continue;
				case LLRP::RC_RecvIOError:
					/// @fixme  close connection more elegantly. (not using progOpt to get ReaderInitiatedOpt)
					READER_LOG (LOGTYPE_ERROR, "IO error is received from the server.\n");
					this->m_initLock.unlock();
					this->m_connMgr->closeConnectionToFnC(this->m_progOpt->getReaderInitiatedOpt());
					continue;
				default: // If there was error in message, continue to LLRP message handling process
					READER_LOG (LOGTYPE_ERROR,
							"Miscellaneous error was occurred while receiving message: %s\n", pError->m_pWhatStr);
					// Store the detail of error. It would be handled later, while handling LLRP message.
					m_ErrorDetails.resultCodeAndWhatStr(pError->m_eResultCode, pError->m_pWhatStr);
				}
			}
			// Start handling the message.
			pRsp = new LLRP::CMessage*;
			READER_LOG(LOGTYPE_DEBUG, "Receiving message...: %s\n",
					pCmd->m_pType->m_pName);

			if (pCmd->m_pType == &LLRP::CKEEPALIVE_ACK::s_typeDescriptor) {
				m_connMgr->setReceivedKeepaliveId(pCmd->getMessageID());
				*pRsp = NULL;
			} else if (pCmd->m_pType == &LLRP::CGET_READER_CAPABILITIES::s_typeDescriptor) {
				handleGetReaderCapabilities(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CADD_ROSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleAddROSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CDELETE_ROSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleDeleteROSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CSTART_ROSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleStartROSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CSTOP_ROSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleStopROSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CENABLE_ROSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleEnableROSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CDISABLE_ROSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleDisableROSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CGET_ROSPECS::s_typeDescriptor) {
				handleGetROSpecs(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CADD_ACCESSSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleAddAccessSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CDELETE_ACCESSSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleDeleteAccessSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CENABLE_ACCESSSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleEnableAccessSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CDISABLE_ACCESSSPEC::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleDisableAccessSpec(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CGET_ACCESSSPECS::s_typeDescriptor) {
				handleGetAccessSpecs(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CGET_READER_CONFIG::s_typeDescriptor) {
				handleGetReaderConfig(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CSET_READER_CONFIG::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleSetReaderConfig(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CCLOSE_CONNECTION::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleCloseConnection(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CGET_SUPPORTED_VERSION::s_typeDescriptor) {
				handleGetSupportedVersion(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CSET_PROTOCOL_VERSION::s_typeDescriptor) {
				m_LLRPConfig->invalidateStateValue();
				handleSetProtocolVersion(pCmd, pRsp);
			} else if (pCmd->m_pType == &LLRP::CERROR_MESSAGE::s_typeDescriptor) {
				*pRsp = NULL;
			} else {
				READER_LOG(LOGTYPE_ERROR, "Unsupported message is received: %s\n",
						pCmd->m_pType->m_pName);
				LLRP::CERROR_MESSAGE *pErrorMsg = new LLRP::CERROR_MESSAGE();
				LLRP::CLLRPStatus *pStatus = new LLRP::CLLRPStatus();
				pStatus->setStatusCode(LLRP::StatusCode_M_UnsupportedMessage);
				pErrorMsg->setLLRPStatus(pStatus);
				*pRsp = (LLRP::CMessage *) pErrorMsg;
			}

			// End handling message contents -----------------------------------
			//this->printXMLMessage(pRsp);
			if (*pRsp != NULL) {
				m_connMgr->sendMessage(*pRsp);
			}


			/*
			 * Done with the message
			 */
			delete pCmd;
			if (*pRsp != NULL) {
				delete *pRsp;
			}
			delete pRsp;

		}


		/*
		 * Victory.
		 */

		this->m_initLock.unlock();
		if (m_shutdown == 1) {
			READER_LOG (LOGTYPE_TRACE, "shutdown variable is set to 1.\n");
			break;
		}
		if (m_receivedCloseConnection == 1) {
			m_receivedCloseConnection = 0;
			// save the reference of stub reader app
			//this->clear();
			this->init();
		}

	}
	m_shutdown = -1;
	return 0;
}

int ELFIN::LLRPCore::handleGetSupportedVersion(LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CGET_SUPPORTED_VERSION* pCmd = (LLRP::CGET_SUPPORTED_VERSION*) _pCmd;
	LLRP::CGET_SUPPORTED_VERSION_RESPONSE* pRsp = new LLRP::CGET_SUPPORTED_VERSION_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		pRsp->setCurrentVersion(2);
		pRsp->setSupportedVersion(2);
	}
	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());
	READER_LOG(LOGTYPE_INFO, "GET_SUPPORTED_VERSION is finished\n");

	return pResult.first;
}

int ELFIN::LLRPCore::handleSetProtocolVersion(LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CSET_PROTOCOL_VERSION* pCmd = (LLRP::CSET_PROTOCOL_VERSION*) _pCmd;
	LLRP::CSET_PROTOCOL_VERSION_RESPONSE* pRsp = new LLRP::CSET_PROTOCOL_VERSION_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (pCmd->getProtocolVersion() == 1) {
			/// @fixme  LLRP 1.0.1 is currently not supported.
			READER_LOG (LOGTYPE_ERROR, "LLRP 1.0.1 is not supported.\n");
			pResult.first = LLRP::StatusCode_M_UnsupportedVersion;
			pResult.second = Utils::getUTF8Str("LLRP 1.0.1 is not supported");
		} else {
			/// @fixme  If this message comes with same protocol version twice,
			/// then LLRP Wrapper should response with M_UnexpectedMessage.
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "GET_SUPPORTED_VERSION is finished\n");

	return pResult.first;
}

int ELFIN::LLRPCore::handleGetReaderCapabilities(LLRP::CMessage* _pCmd,
		LLRP::CMessage** _pRsp) {
	LLRP::CGET_READER_CAPABILITIES* pCmd = (LLRP::CGET_READER_CAPABILITIES*) _pCmd;
	LLRP::CGET_READER_CAPABILITIES_RESPONSE* pRsp = new LLRP::CGET_READER_CAPABILITIES_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		LLRP::EGetReaderCapabilitiesRequestedData reqData = pCmd->getRequestedData();
		if (reqData == LLRP::GetReaderCapabilitiesRequestedData_General_Device_Capabilities ||
				reqData == LLRP::GetReaderCapabilitiesRequestedData_All) {
			LLRP::CGeneralDeviceCapabilities *pCap = new LLRP::CGeneralDeviceCapabilities();
			pCap->setDeviceManufacturerName(CAP_IANAPEN);
			pCap->setModelName(CAP_MODEL_NAME);
			pCap->setReaderFirmwareVersion(Utils::getUTF8Str(CAP_FIRMWARE_VER_STR));
			pCap->setMaxNumberOfAntennaSupported(CAP_MAX_ANTENNAS);
			pCap->setCanSetAntennaProperties(CAP_CAN_SET_ANTENNA_PROPERTIES);
			// MaximumReceiveTable is optional, so omitted.
			// TODO: Add MaximumReceiveTable parameter to LTK definition file
			LLRP::CReceiveSensitivityTableEntry *pEntry = new LLRP::CReceiveSensitivityTableEntry();
			// Assume that reader does not allow control of receive sensitivity. (LLRP 1.1, 10.2.1.2)
			if (CAP_ALLOW_CONTROL_OF_RECEIVE_SENSITIVITY == 0) {
				pEntry->setIndex(0);
				pEntry->setReceiveSensitivityValue(0);
				pCap->addReceiveSensitivityTableEntry(pEntry);
			}
			else {
				// TODO: Add sensitivity table entries, for the case of the reader allows control of receive sensitivity
			}
			for (AntennaMap::iterator airIter = m_physicalReader->beginAntennaMap();
					airIter != m_physicalReader->endAntennaMap();airIter++) {
				StubAntenna *pAntenna = (*airIter).second;
				/* CPerAntennaReceiveSensitivityRange is optional, so omitted.
				LLRP::CPerAntennaReceiveSensitivityRange *pPerAntennaRevSensRange = new LLRP::CPerAntennaReceiveSensitivityRange();
				*/
				LLRP::CPerAntennaAirProtocol *pPerAntennaAirProtocol = new LLRP::CPerAntennaAirProtocol();
				pPerAntennaAirProtocol->setAntennaID(pAntenna->_pAntennaID);
				LLRP::llrp_u8v_t *protocolIDU8Str = new LLRP::llrp_u8v_t(1);
				// 1 means EPCglobal Class 1 Gen 2. Just hard-coded for this currently.
				protocolIDU8Str->m_pValue[0] = 1;
				pPerAntennaAirProtocol->setProtocolID(*protocolIDU8Str);
				delete protocolIDU8Str;
				pCap->addPerAntennaAirProtocol(pPerAntennaAirProtocol);
			}
			LLRP::CGPIOCapabilities *pGPICap = new LLRP::CGPIOCapabilities();
			pGPICap->setNumGPIs(this->m_progOpt->getGPIPortCount());
			pGPICap->setNumGPOs(this->m_progOpt->getGPOPortCount());
			pCap->setGPIOCapabilities(pGPICap);
			pRsp->setGeneralDeviceCapabilities(pCap);
		} if (reqData == LLRP::GetReaderCapabilitiesRequestedData_LLRP_Capabilities ||
				reqData == LLRP::GetReaderCapabilitiesRequestedData_All) {
			LLRP::CLLRPCapabilities *pCap = new LLRP::CLLRPCapabilities();
			pCap->setCanDoRFSurvey(CAP_CAN_DO_RFSURVEY);
			pCap->setCanDoTagInventoryStateAwareSingulation(CAP_CAN_DO_TAG_INVENTROY_STATE_AWARE_SINGULATION);
			pCap->setCanReportBufferFillWarning(CAP_CAN_REPORT_BUFFER_FILL_WARNING);
			pCap->setMaxNumROSpecs(CAP_MAX_NUM_ROSPECS);
			pCap->setMaxNumSpecsPerROSpec(CAP_MAX_NUM_SPECS_PER_ROSPEC);
			pCap->setMaxNumInventoryParameterSpecsPerAISpec(CAP_MAX_NUM_INV_PAR_SPECS_PER_AISPEC);
			pCap->setMaxNumPriorityLevelsSupported(CAP_MAX_PRIORITY_LEVEL_SUPPORTED);
			pCap->setMaxNumAccessSpecs(CAP_MAX_NUM_ACCESSSPECS);
			pCap->setMaxNumOpSpecsPerAccessSpec(CAP_MAX_NUM_OPSPECS_PER_ACCESSSPEC);
			pCap->setSupportsClientRequestOpSpec(CAP_SUPPORTS_CLIENT_REQUEST_OPSPEC);
			pCap->setClientRequestOpSpecTimeout(CAP_CLIENT_REQUEST_OPSPEC_TIMEOUT);
			pCap->setSupportsEventAndReportHolding(CAP_SUPPORTS_EVENT_AND_REPORT_HOLDING);
			pRsp->setLLRPCapabilities(pCap);
		} if (reqData == LLRP::GetReaderCapabilitiesRequestedData_Regulatory_Capabilities ||
				reqData == LLRP::GetReaderCapabilitiesRequestedData_All) {
			LLRP::CRegulatoryCapabilities *pReg = new LLRP::CRegulatoryCapabilities();
			pReg->setCountryCode(CAP_COUNTRY_CODE);
			pReg->setCommunicationsStandard((LLRP::ECommunicationsStandard) CAP_COMM_STANDARD);
			// UHFBandCapabilities is optional, so omitted
			pRsp->setRegulatoryCapabilities(pReg);

		} if (reqData == LLRP::GetReaderCapabilitiesRequestedData_LLRP_Air_Protocol_Capabilities ||
				reqData == LLRP::GetReaderCapabilitiesRequestedData_All) {
			LLRP::CC1G2LLRPCapabilities *pLLRPCaps = new LLRP::CC1G2LLRPCapabilities();
			pLLRPCaps->setCanSupportBlockErase(CAP_CAN_SUPPORT_BLOCKERASE);
			pLLRPCaps->setCanSupportBlockWrite(CAP_CAN_SUPPORT_BLOCKWRITE);
			pLLRPCaps->setCanSupportBlockPermalock(CAP_CAN_SUPPORT_BLOCKPERMALOCK);
			pLLRPCaps->setCanSupportTagRecommissioning(CAP_CAN_SUPPORT_TAGRECOMMISSIONING);
			pLLRPCaps->setCanSupportUMIMethod2(CAP_CAN_SUPPORT_UMIMETHOD2);
			pLLRPCaps->setCanSupportXPC(CAP_CAN_SUPPORT_XPC);
			pLLRPCaps->setMaxNumSelectFiltersPerQuery(CAP_MAX_NUM_SELECT_FILTERS_PER_QUERY);
			pRsp->setAirProtocolLLRPCapabilities((LLRP::CParameter *) pLLRPCaps);
		} if (reqData > 4) {
			// EGetReaderCapabilitiesRequestedData is defined up to 4, so this is illegal
			pResult = Result_t(LLRP::StatusCode_M_FieldError, Utils::getUTF8Str("RequestedData field is invalid"));
		}
	}


	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "GET_READER_CAPABILITIES is finished\n");

	return pResult.first;
}

int ELFIN::LLRPCore::handleAddROSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CADD_ROSPEC* pCmd = (LLRP::CADD_ROSPEC*) _pCmd;
	LLRP::CADD_ROSPEC_RESPONSE* pRsp = new LLRP::CADD_ROSPEC_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		// To use ROSpec in the given command, remove it from sub parameter list of _pCmd.
		// If we don't remove this, it would be deleted together, when pCmd is deleted.
		pCmd->removeSubParameterFromAllList((LLRP::CParameter *) pCmd->getROSpec());
		if (m_ROAdmin->addROSpec(pCmd->getROSpec()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to add ROSpec"));
		}
	}
	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "ADD_ROSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleDeleteROSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CDELETE_ROSPEC* pCmd = (LLRP::CDELETE_ROSPEC*) _pCmd;
	LLRP::CDELETE_ROSPEC_RESPONSE* pRsp = new LLRP::CDELETE_ROSPEC_RESPONSE();
	*_pRsp = pRsp;

	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (m_ROAdmin->deleteROSpec(pCmd->getROSpecID()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to delete ROSpec"));
		}
	}
	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "DELETE_ROSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleStartROSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CSTART_ROSPEC* pCmd = (LLRP::CSTART_ROSPEC*) _pCmd;
	LLRP::CSTART_ROSPEC_RESPONSE* pRsp = new LLRP::CSTART_ROSPEC_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (m_ROAdmin->startROSpec(pCmd->getROSpecID()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to start ROSpec"));
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "START_ROSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleStopROSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CSTOP_ROSPEC* pCmd = (LLRP::CSTOP_ROSPEC*) _pCmd;
	LLRP::CSTOP_ROSPEC_RESPONSE* pRsp = new LLRP::CSTOP_ROSPEC_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (m_ROAdmin->stopROSpec(pCmd->getROSpecID()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to stop ROSpec"));
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "STOP_ROSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleEnableROSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CENABLE_ROSPEC* pCmd = (LLRP::CENABLE_ROSPEC*) _pCmd;
	LLRP::CENABLE_ROSPEC_RESPONSE* pRsp = new LLRP::CENABLE_ROSPEC_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (m_ROAdmin->enableROSpec(pCmd->getROSpecID()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to enable ROSpec"));
		}
	}
	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "ENABLE_ROSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleDisableROSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CDISABLE_ROSPEC* pCmd = (LLRP::CDISABLE_ROSPEC*) _pCmd;
	LLRP::CDISABLE_ROSPEC_RESPONSE* pRsp = new LLRP::CDISABLE_ROSPEC_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (m_ROAdmin->disableROSpec(pCmd->getROSpecID()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to disable ROSpec"));
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "DISABLE_ROSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleGetROSpecs(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CGET_ROSPECS* pCmd = (LLRP::CGET_ROSPECS*) _pCmd;
	LLRP::CGET_ROSPECS_RESPONSE* pRsp = new LLRP::CGET_ROSPECS_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		// Create vector of pointer to current ROSpecs.
		std::vector<LLRP::CROSpec*> *ROSpecs = m_ROAdmin->getROSpecs();
		for (unsigned int i = 0;i < ROSpecs->size();i++) {
			pRsp->addROSpec(ROSpecs->back());
			// If we don't remove each ROSpec from sub parameter list of pRsp,
			// they would be deleted together, when pRsp is deleted.
			pRsp->removeSubParameterFromAllList(ROSpecs->back());
			ROSpecs->pop_back();
		}
		delete ROSpecs;
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "GET_ROSPECS is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleAddAccessSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CADD_ACCESSSPEC* pCmd = (LLRP::CADD_ACCESSSPEC*) _pCmd;
	LLRP::CADD_ACCESSSPEC_RESPONSE* pRsp = new LLRP::CADD_ACCESSSPEC_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		// To use AccessSpec in the given command, remove it from sub parameter list of _pCmd.
		// If we don't remove this, it would be deleted together, when pCmd is deleted.
		pCmd->removeSubParameterFromAllList(pCmd->getAccessSpec());
		if (m_AOAdmin->addAccessSpec(pCmd->getAccessSpec()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to add AccessSpec"));
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "ADD_ACCESSSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleDeleteAccessSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CDELETE_ACCESSSPEC* pCmd = (LLRP::CDELETE_ACCESSSPEC*) _pCmd;
	LLRP::CDELETE_ACCESSSPEC_RESPONSE* pRsp = new LLRP::CDELETE_ACCESSSPEC_RESPONSE();
	*_pRsp = pRsp;

	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (m_AOAdmin->deleteAccessSpec(pCmd->getAccessSpecID()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to delete AccessSpec"));
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "DELETE_ACCESSSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleEnableAccessSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CENABLE_ACCESSSPEC* pCmd = (LLRP::CENABLE_ACCESSSPEC*) _pCmd;
	LLRP::CENABLE_ACCESSSPEC_RESPONSE* pRsp = new LLRP::CENABLE_ACCESSSPEC_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (m_AOAdmin->enableAccessSpec(pCmd->getAccessSpecID()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to enable AccessSpec"));
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);


	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "ENABLE_ACCESSSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleDisableAccessSpec(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CDISABLE_ACCESSSPEC* pCmd = (LLRP::CDISABLE_ACCESSSPEC*) _pCmd;
	LLRP::CDISABLE_ACCESSSPEC_RESPONSE* pRsp = new LLRP::CDISABLE_ACCESSSPEC_RESPONSE();
	*_pRsp = pRsp;
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		if (m_AOAdmin->disableAccessSpec(pCmd->getAccessSpecID()) != 0) {
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Failed to disable AccessSpec"));
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "DISABLE_ACCESSSPEC is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleGetAccessSpecs(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CGET_ACCESSSPECS* pCmd = (LLRP::CGET_ACCESSSPECS*) _pCmd;
	LLRP::CGET_ACCESSSPECS_RESPONSE* pRsp = new LLRP::CGET_ACCESSSPECS_RESPONSE();
	*_pRsp = pRsp;

	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		std::vector<LLRP::CAccessSpec*> *AccessSpecs = m_AOAdmin->getAccessSpecs();
		for (unsigned int i = 0;i < AccessSpecs->size();i++) {
			pRsp->addAccessSpec(AccessSpecs->back());
			// If we don't remove each AccessSpec from sub parameter list of pRsp,
			// they would be deleted together, when pRsp is deleted.
			pRsp->removeSubParameterFromAllList(AccessSpecs->back());
			AccessSpecs->pop_back();
		}
		delete AccessSpecs;
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "GET_ACCESSSPECS is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleGetReaderConfig(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CGET_READER_CONFIG* pCmd = (LLRP::CGET_READER_CONFIG*) _pCmd;
	LLRP::CGET_READER_CONFIG_RESPONSE* pRsp = new LLRP::CGET_READER_CONFIG_RESPONSE();
	*_pRsp = pRsp;

	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		LLRP::EGetReaderConfigRequestedData reqData = pCmd->getRequestedData();
		if ((reqData == LLRP::GetReaderConfigRequestedData_Identification
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::llrp_u8v_t *readerID;
			LLRP::CIdentification *pIdt;
			// Identification
			pIdt = new LLRP::CIdentification();
			pIdt->setIDType(LLRP::IdentificationType_EPC);
			readerID = new LLRP::llrp_u8v_t(12);
			for (int j = 0; j < 12; j++) {
				readerID->m_pValue[j] = m_physicalReader->m_ReaderEPC->m_aValue[j];
			}
			pIdt->setReaderID(*readerID);
			delete readerID;
			pRsp->setIdentification(pIdt);
		} if ((reqData == LLRP::GetReaderConfigRequestedData_AntennaProperties
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CAntennaProperties *pAntProp = NULL;
			for (AntennaMap::iterator iter = m_physicalReader->beginAntennaMap();
					iter != m_physicalReader->endAntennaMap();iter++) {
				if (pCmd->getAntennaID() != 0 && (*iter).second->_pAntennaID != pCmd->getAntennaID())
					continue; // continue until the requested antenna ID shows
				pAntProp = new LLRP::CAntennaProperties();
				pAntProp->setAntennaID(
						(LLRP::llrp_u16_t) (*iter).second->_pAntennaID);
				pAntProp->setAntennaConnected(
						(LLRP::llrp_u1_t) (*iter).second->_pAntennaConnected);
				pAntProp->setAntennaGain(
						(LLRP::llrp_u1_t) (*iter).second->_pAntennaGain);
				pRsp->addAntennaProperties(pAntProp);
				pAntProp = NULL;
			}
			if (pCmd->getAntennaID() != 0 && pRsp->countAntennaProperties() == 0) {
				pResult = Result_t(
						LLRP::StatusCode_M_FieldError,
						Utils::getUTF8Str("Antenna does not exist"));
				READER_LOG(LOGTYPE_ERROR, "AntennaProperties: Antenna ID (%d) does not exist.\n", pCmd->getAntennaID());
			}
		} if ((reqData == LLRP::GetReaderConfigRequestedData_AntennaConfiguration
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CAntennaConfiguration *pAntConf = NULL;
			for (AntennaMap::iterator iter = m_physicalReader->beginAntennaMap();
					iter != m_physicalReader->endAntennaMap();iter++) {
				if (pCmd->getAntennaID() != 0 && (*iter).second->_pAntennaID != pCmd->getAntennaID())
					continue; // continue until the requested antenna ID shows
				pAntConf = new LLRP::CAntennaConfiguration();
				pAntConf->setAntennaID(
						(LLRP::llrp_u16_t) (*iter).second->_pAntennaID);
				// RFReceiver and RFTransmitter are currently just set as 0
				LLRP::CRFReceiver *pRFRecv = new LLRP::CRFReceiver();
				pRFRecv->setReceiverSensitivity((*iter).second->_ReceiverSensitivity);
				pAntConf->setRFReceiver(pRFRecv);
				LLRP::CRFTransmitter *pRFTrans = new LLRP::CRFTransmitter();
				pRFTrans->setChannelIndex((*iter).second->_ChannelIndex);
				pRFTrans->setHopTableID((*iter).second->_HopTableID);
				pRFTrans->setTransmitPower((*iter).second->_pTransmitPower);
				pAntConf->setRFTransmitter(pRFTrans);
				pRsp->addAntennaConfiguration(pAntConf);
				pAntConf = NULL;
			}
			if (pCmd->getAntennaID() != 0 && pRsp->countAntennaConfiguration() == 0) {
				pResult = Result_t(
						LLRP::StatusCode_M_FieldError,
						Utils::getUTF8Str("Antenna does not exist"));
				READER_LOG(LOGTYPE_ERROR, "AntennaConfiguration: Antenna ID (%d) does not exist.\n", pCmd->getAntennaID());
			}
		} if ((reqData == LLRP::GetReaderConfigRequestedData_ROReportSpec
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CROReportSpec *pROReportSpec = new LLRP::CROReportSpec();
			pROReportSpec->setROReportTrigger(m_LLRPConfig->_pDefaultROReportSpec->getROReportTrigger());
			pROReportSpec->setN(m_LLRPConfig->_pDefaultROReportSpec->getN());
			LLRP::CTagReportContentSelector *pSelector = new LLRP::CTagReportContentSelector();
			pSelector->setEnableAccessSpecID(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableAccessSpecID());
			pSelector->setEnableAntennaID(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableAntennaID());
			pSelector->setEnableChannelIndex(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableChannelIndex());
			pSelector->setEnableFirstSeenTimestamp(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableFirstSeenTimestamp());
			pSelector->setEnableInventoryParameterSpecID(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableInventoryParameterSpecID());
			pSelector->setEnableLastSeenTimestamp(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableLastSeenTimestamp());
			pSelector->setEnablePeakRSSI(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnablePeakRSSI());
			pSelector->setEnableROSpecID(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableROSpecID());
			pSelector->setEnableSpecIndex(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableSpecIndex());
			pSelector->setEnableTagSeenCount(m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->getEnableTagSeenCount());
			if (m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->countAirProtocolEPCMemorySelector() > 0) {
				// Only first element is meaningful. Ignore others.
				LLRP::CC1G2EPCMemorySelector *pMemSelector = new LLRP::CC1G2EPCMemorySelector();
				LLRP::CC1G2EPCMemorySelector *pMemSelector_source = (LLRP::CC1G2EPCMemorySelector *) (*m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector());
				pMemSelector->setEnableCRC(pMemSelector_source->getEnableCRC());
				pMemSelector->setEnablePCBits(pMemSelector_source->getEnablePCBits());
				pSelector->addAirProtocolEPCMemorySelector((LLRP::CParameter *) pMemSelector);
			}
			pROReportSpec->setTagReportContentSelector(pSelector);
			pRsp->setROReportSpec(pROReportSpec);
		} if ((reqData == LLRP::GetReaderConfigRequestedData_ReaderEventNotificationSpec
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CReaderEventNotificationSpec *ReaderEventNotificationSpec = new LLRP::CReaderEventNotificationSpec();
			for (std::list<LLRP::CEventNotificationState *>::iterator def_iter = m_LLRPConfig->_pDefaultReaderEventNotificationSpec->beginEventNotificationState();
					def_iter != m_LLRPConfig->_pDefaultReaderEventNotificationSpec->endEventNotificationState();def_iter++) {
				LLRP::CEventNotificationState *EventNotificationState = new LLRP::CEventNotificationState();
				EventNotificationState->setEventType((*def_iter)->getEventType());
				EventNotificationState->setNotificationState((*def_iter)->getNotificationState());
				ReaderEventNotificationSpec->addEventNotificationState(EventNotificationState);
			}
			pRsp->setReaderEventNotificationSpec(ReaderEventNotificationSpec);
		} if ((reqData == LLRP::GetReaderConfigRequestedData_AccessReportSpec
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CAccessReportSpec *pAccessReportSpec = new LLRP::CAccessReportSpec();
			pAccessReportSpec->setAccessReportTrigger(m_LLRPConfig->_pDefaultAccessReportSpec->getAccessReportTrigger());
			pRsp->setAccessReportSpec(pAccessReportSpec);
		} if ((reqData == LLRP::GetReaderConfigRequestedData_LLRPConfigurationStateValue
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CLLRPConfigurationStateValue *LLRPConfigurationStateValue = new LLRP::CLLRPConfigurationStateValue();
			LLRPConfigurationStateValue->setLLRPConfigurationStateValue(m_LLRPConfig->_pLLRPConfigurationStateValue);
			pRsp->setLLRPConfigurationStateValue(LLRPConfigurationStateValue);
		} if ((reqData == LLRP::GetReaderConfigRequestedData_KeepaliveSpec
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CKeepaliveSpec *pKeepaliveSpec = new LLRP::CKeepaliveSpec();
			pKeepaliveSpec->setKeepaliveTriggerType(m_LLRPConfig->_pDefaultKeepaliveSpec->getKeepaliveTriggerType());
			pKeepaliveSpec->setPeriodicTriggerValue(m_LLRPConfig->_pDefaultKeepaliveSpec->getPeriodicTriggerValue());
			pRsp->setKeepaliveSpec(pKeepaliveSpec);
		} if ((reqData == LLRP::GetReaderConfigRequestedData_GPIPortCurrentState
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CGPIPortCurrentState *pGPIPortCurrentState = NULL;
			for (GPIPortMap::iterator iter = m_physicalReader->beginGPIPortMap();
					iter != m_physicalReader->endGPIPortMap();iter++) {
				if (pCmd->getGPIPortNum() != 0 && (*iter).second->getPortNum() != pCmd->getGPIPortNum())
					continue; // continue until the requested antenna ID shows
				pGPIPortCurrentState = new LLRP::CGPIPortCurrentState();
				pGPIPortCurrentState->setGPIPortNum((*iter).second->getPortNum());
				pGPIPortCurrentState->setConfig((*iter).second->getConfig());
				pGPIPortCurrentState->setState((*iter).second->getState());
				pRsp->addGPIPortCurrentState(pGPIPortCurrentState);
				pGPIPortCurrentState = NULL;
			}
			if (pCmd->getGPIPortNum() != 0 && pRsp->countGPIPortCurrentState() == 0) {
				pResult = Result_t(
						LLRP::StatusCode_M_FieldError,
						Utils::getUTF8Str("GPI port does not exist"));
				READER_LOG(LOGTYPE_ERROR, "GPIPortCurrentState: GPI port ID (%d) does not exist.\n", pCmd->getGPIPortNum());
			}
		} if ((reqData == LLRP::GetReaderConfigRequestedData_GPOWriteData
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CGPOWriteData *_GPOWriteData = NULL;
			for (GPOPortMap::iterator iter = m_physicalReader->beginGPOPortMap();
					iter != m_physicalReader->endGPOPortMap();iter++) {
				if (pCmd->getGPOPortNum() != 0 && (*iter).second->getPortNum() != pCmd->getGPOPortNum())
					continue; // continue until the requested antenna ID shows
				_GPOWriteData = new LLRP::CGPOWriteData();
				_GPOWriteData->setGPOPortNumber((*iter).second->getPortNum());
				_GPOWriteData->setGPOData((*iter).second->getData());
				pRsp->addGPOWriteData(_GPOWriteData);
				_GPOWriteData = NULL;
			}
			if (pCmd->getGPOPortNum() != 0 && pRsp->countGPOWriteData() == 0) {
				pResult = Result_t(
						LLRP::StatusCode_M_FieldError,
						Utils::getUTF8Str("GPO port does not exist"));
				READER_LOG(LOGTYPE_ERROR, "GPOWriteData: GPO port ID (%d) does not exist.\n", pCmd->getGPOPortNum());
			}
		} if ((reqData == LLRP::GetReaderConfigRequestedData_EventsAndReports
				|| reqData == LLRP::GetReaderConfigRequestedData_All) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CEventsAndReports *pEventsAndReports = new LLRP::CEventsAndReports();
			pEventsAndReports->setHoldEventsAndReportsUponReconnect(m_LLRPConfig->_pDefaultEventsAndReports->getHoldEventsAndReportsUponReconnect());
			pRsp->setEventsAndReports(pEventsAndReports);
		}

		if (reqData > 11 && pResult.first == LLRP::StatusCode_M_Success) {
			// enum EGetReaderConfigRequestedData ends at 11, so this is illegal
			pResult = Result_t(
					LLRP::StatusCode_M_FieldError,
					Utils::getUTF8Str("Requested data field contains illegal value"));
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "GET_READER_CONFIG is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleSetReaderConfig(LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CSET_READER_CONFIG* pCmd = (LLRP::CSET_READER_CONFIG*) _pCmd;
	LLRP::CSET_READER_CONFIG_RESPONSE* pRsp = new LLRP::CSET_READER_CONFIG_RESPONSE();
	*_pRsp = pRsp;
	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		// Check whether to reset as factory default
		LLRP::llrp_u1_t bReset = pCmd->getResetToFactoryDefault();
		// reset to factory default, which is defined in config.llrp
		if (bReset != 0) {
			this->loadDefaultConfigFile();
		}
		if ((pCmd->getReaderEventNotificationSpec() != NULL) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CReaderEventNotificationSpec *pReaderEventNotificationSpec = pCmd->getReaderEventNotificationSpec();
			for (std::list<LLRP::CEventNotificationState *>::iterator spec_iter = pReaderEventNotificationSpec->beginEventNotificationState();
					spec_iter != pReaderEventNotificationSpec->endEventNotificationState();spec_iter++) {
				for (std::list<LLRP::CEventNotificationState *>::iterator def_iter = m_LLRPConfig->_pDefaultReaderEventNotificationSpec->beginEventNotificationState();
						def_iter != m_LLRPConfig->_pDefaultReaderEventNotificationSpec->endEventNotificationState();def_iter++) {
					if ((*spec_iter)->getEventType() == (*def_iter)->getEventType()) {
						(*def_iter)->setNotificationState((*spec_iter)->getNotificationState());
						m_LLRPConfig->_pEventNotiArray[(*def_iter)->getEventType()] = (*spec_iter)->getNotificationState();
						break;
					}
				}
			}
		}
		if ((pCmd->countAntennaProperties() != 0) && pResult.first == LLRP::StatusCode_M_Success) {
			// Iterate AntennaProperties in the command
			for (std::list<LLRP::CAntennaProperties *>::iterator prop_iter = pCmd->beginAntennaProperties();
					prop_iter != pCmd->endAntennaProperties();prop_iter++) {
				AbstractAntenna *pAntenna = NULL;
				// Iterate AbstractAntenna to find target antenna using antenna ID
				for (AntennaMap::iterator ant_iter = m_physicalReader->beginAntennaMap();
						ant_iter != m_physicalReader->endAntennaMap();ant_iter++) {
					if ((*ant_iter).second->_pAntennaID != (*prop_iter)->getAntennaID())
						continue; // continue until the requested antenna ID shows
					(*ant_iter).second->_pAntennaConnected = (*prop_iter)->getAntennaConnected();
					(*ant_iter).second->_pAntennaGain = (*prop_iter)->getAntennaGain();

				}
				// If the antenna ID is not found, then handle the error
				if (pAntenna == NULL) {
					pResult = Result_t(
							LLRP::StatusCode_M_FieldError,
							Utils::getUTF8Str("Antenna does not exist"));
					READER_LOG(LOGTYPE_ERROR, "AntennaProperties: Antenna ID (%d) does not exist.\n", (*prop_iter)->getAntennaID());
				}
			}
		} if ((pCmd->countAntennaConfiguration() != 0) && pResult.first == LLRP::StatusCode_M_Success) {
			for (std::list<LLRP::CAntennaConfiguration *>::iterator conf_iter = pCmd->beginAntennaConfiguration();
					conf_iter != pCmd->endAntennaConfiguration();conf_iter++) {
				AbstractAntenna *pAntenna = NULL;
				// Iterate AbstractAntenna to find target antenna using antenna ID
				for (AntennaMap::iterator ant_iter = m_physicalReader->beginAntennaMap();
						ant_iter != m_physicalReader->endAntennaMap();ant_iter++) {
					if ((*ant_iter).second->_pAntennaID != (*conf_iter)->getAntennaID())
						continue; // continue until the requested antenna ID shows
					if ((*conf_iter)->getRFReceiver() != NULL) {
						(*ant_iter).second->_ReceiverSensitivity = (*conf_iter)->getRFReceiver()->getReceiverSensitivity();
					}
					if ((*conf_iter)->getRFTransmitter() != NULL) {
						(*ant_iter).second->_ChannelIndex = (*conf_iter)->getRFTransmitter()->getChannelIndex();
						(*ant_iter).second->_HopTableID = (*conf_iter)->getRFTransmitter()->getHopTableID();
						(*ant_iter).second->setTransmitPower((*conf_iter)->getRFTransmitter()->getTransmitPower());
					}
				}
				// If the antenna ID is not found, then handle the error
				if (pAntenna == NULL) {
					pResult = Result_t(
							LLRP::StatusCode_M_FieldError,
							Utils::getUTF8Str("Antenna does not exist"));
					READER_LOG(LOGTYPE_ERROR, "AntennaConfiguration: Antenna ID (%d) does not exist.\n", (*conf_iter)->getAntennaID());
				}
			}

		} if ((pCmd->getROReportSpec() != NULL) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CROReportSpec *pROReportSpec = pCmd->getROReportSpec();
			m_LLRPConfig->_pDefaultROReportSpec->setROReportTrigger(pROReportSpec->getROReportTrigger());
			m_LLRPConfig->_pDefaultROReportSpec->setN(pROReportSpec->getN());
			LLRP::CTagReportContentSelector *_pSelector = m_LLRPConfig->_pDefaultROReportSpec->getTagReportContentSelector();
			_pSelector->setEnableAccessSpecID(pROReportSpec->getTagReportContentSelector()->getEnableAccessSpecID());
			_pSelector->setEnableAntennaID(pROReportSpec->getTagReportContentSelector()->getEnableAntennaID());
			_pSelector->setEnableChannelIndex(pROReportSpec->getTagReportContentSelector()->getEnableChannelIndex());
			_pSelector->setEnableFirstSeenTimestamp(pROReportSpec->getTagReportContentSelector()->getEnableFirstSeenTimestamp());
			_pSelector->setEnableInventoryParameterSpecID(pROReportSpec->getTagReportContentSelector()->getEnableInventoryParameterSpecID());
			_pSelector->setEnableLastSeenTimestamp(pROReportSpec->getTagReportContentSelector()->getEnableLastSeenTimestamp());
			_pSelector->setEnablePeakRSSI(pROReportSpec->getTagReportContentSelector()->getEnablePeakRSSI());
			_pSelector->setEnableROSpecID(pROReportSpec->getTagReportContentSelector()->getEnableROSpecID());
			_pSelector->setEnableSpecIndex(pROReportSpec->getTagReportContentSelector()->getEnableSpecIndex());
			_pSelector->setEnableTagSeenCount(pROReportSpec->getTagReportContentSelector()->getEnableTagSeenCount());
			if (pCmd->getROReportSpec()->getTagReportContentSelector()->countAirProtocolEPCMemorySelector() > 0) {
				// Only first element is meaningful. Ignore others.
				if (_pSelector->countAirProtocolEPCMemorySelector() > 0) {
					LLRP::CC1G2EPCMemorySelector *pMemSelector = (LLRP::CC1G2EPCMemorySelector *) (*_pSelector->beginAirProtocolEPCMemorySelector());
					LLRP::CC1G2EPCMemorySelector *pMemSelector_source = (LLRP::CC1G2EPCMemorySelector *) (*pROReportSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector());
					pMemSelector->setEnableCRC(pMemSelector_source->getEnableCRC());
					pMemSelector->setEnablePCBits(pMemSelector_source->getEnablePCBits());
				}
				else {
					LLRP::CC1G2EPCMemorySelector *pMemSelector = new LLRP::CC1G2EPCMemorySelector();
					LLRP::CC1G2EPCMemorySelector *pMemSelector_source = (LLRP::CC1G2EPCMemorySelector *) (*pROReportSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector());
					pMemSelector->setEnableCRC(pMemSelector_source->getEnableCRC());
					pMemSelector->setEnablePCBits(pMemSelector_source->getEnablePCBits());
					_pSelector->addAirProtocolEPCMemorySelector((LLRP::CParameter *) pMemSelector);
				}
			}
			else {
				_pSelector->clearAirProtocolEPCMemorySelector();
			}
		} if ((pCmd->getAccessReportSpec() != NULL) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CAccessReportSpec *pAccessReportSpec = pCmd->getAccessReportSpec();
			m_LLRPConfig->_pDefaultAccessReportSpec->setAccessReportTrigger(pAccessReportSpec->getAccessReportTrigger());
		} if ((pCmd->getKeepaliveSpec() != NULL) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CKeepaliveSpec *pKeepaliveSpec = pCmd->getKeepaliveSpec();
			if (pKeepaliveSpec != NULL && pKeepaliveSpec->getKeepaliveTriggerType() == 1) {
				m_connMgr->startKeepaliveThread(pKeepaliveSpec->getPeriodicTriggerValue());
				m_LLRPConfig->_pDefaultKeepaliveSpec->setKeepaliveTriggerType(pKeepaliveSpec->getKeepaliveTriggerType());
				m_LLRPConfig->_pDefaultKeepaliveSpec->setPeriodicTriggerValue(pKeepaliveSpec->getPeriodicTriggerValue());
			}
			else if (pKeepaliveSpec != NULL && pKeepaliveSpec->getKeepaliveTriggerType() == 0) {
				m_connMgr->stopKeepaliveThread();
				m_LLRPConfig->_pDefaultKeepaliveSpec->setKeepaliveTriggerType(pKeepaliveSpec->getKeepaliveTriggerType());
				m_LLRPConfig->_pDefaultKeepaliveSpec->setPeriodicTriggerValue(pKeepaliveSpec->getPeriodicTriggerValue());
			}
			else if (pKeepaliveSpec != NULL && pKeepaliveSpec->getKeepaliveTriggerType() > 1) {
				pResult = Result_t(
						LLRP::StatusCode_P_ParameterError,
						Utils::getUTF8Str("KeepAliveSpec is invalid"));
			}
		} if ((pCmd->countGPIPortCurrentState() != 0) && pResult.first == LLRP::StatusCode_M_Success) {
			for (std::list<LLRP::CGPIPortCurrentState *>::iterator state_iter = pCmd->beginGPIPortCurrentState();
					state_iter != pCmd->endGPIPortCurrentState();state_iter++) {
				bool port_exists = false;
				for (GPIPortMap::iterator port_iter = m_physicalReader->beginGPIPortMap();
						port_iter != m_physicalReader->endGPIPortMap();port_iter++) {
					if ((*port_iter).second->getPortNum() != (*state_iter)->getGPIPortNum())
						continue; // continue until the requested antenna ID shows
					(*port_iter).second->setConfig((*state_iter)->getConfig());
					port_exists = true;
				}
				if (port_exists == false) {
					pResult = Result_t(
							LLRP::StatusCode_M_FieldError,
							Utils::getUTF8Str("GPI port does not exist"));
					READER_LOG(LOGTYPE_ERROR, "GPIPortCurrentState: GPI port ID (%d) does not exist.\n", (*state_iter)->getGPIPortNum());
				}
			}
		} if ((pCmd->countGPOWriteData() != 0) && pResult.first == LLRP::StatusCode_M_Success) {
			for (std::list<LLRP::CGPOWriteData *>::iterator data_iter = pCmd->beginGPOWriteData();
					data_iter != pCmd->endGPOWriteData();data_iter++) {
				bool port_exists = false;
				for (GPOPortMap::iterator port_iter = m_physicalReader->beginGPOPortMap();
						port_iter != m_physicalReader->endGPOPortMap();port_iter++) {
					if ((*port_iter).second->getPortNum() != (*data_iter)->getGPOPortNumber())
						continue; // continue until the requested antenna ID shows
					(*port_iter).second->setData((*data_iter)->getGPOData());
					port_exists = true;
				}
				if (port_exists == false) {
					pResult = Result_t(
							LLRP::StatusCode_M_FieldError,
							Utils::getUTF8Str("GPO port does not exist"));
					READER_LOG(LOGTYPE_ERROR, "GPOWriteData: GPO port ID (%d) does not exist.\n", (*data_iter)->getGPOPortNumber());
					/// @todo  Handle no such GPOPort failure
				}
			}
		} if ((pCmd->getEventsAndReports() != NULL) && pResult.first == LLRP::StatusCode_M_Success) {
			LLRP::CEventsAndReports *pEventsAndReports = pCmd->getEventsAndReports();
			m_LLRPConfig->_pDefaultEventsAndReports->setHoldEventsAndReportsUponReconnect(pEventsAndReports->getHoldEventsAndReportsUponReconnect());
		}
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "SET_READER_CONFIG is finished\n");

	return 0;
}

int ELFIN::LLRPCore::handleCloseConnection(LLRP::CMessage * _pCmd, LLRP::CMessage **_pRsp) {
	LLRP::CCLOSE_CONNECTION* pCmd = (LLRP::CCLOSE_CONNECTION*) _pCmd;
	LLRP::CCLOSE_CONNECTION_RESPONSE* pRsp = new LLRP::CCLOSE_CONNECTION_RESPONSE();
	*_pRsp = pRsp;

	Result_t pResult = getErrorResult();
	if (pResult.first == LLRP::StatusCode_M_Success) {
		// This message is used to handle non-standard commands from virtual-ALE for conformance test.
		// In real deployment, you should disable these features.
#if 1
		if (pCmd->getMessageID() == 979797) {
			// If the received message ID is 979797, then just shutdown the LLRP Wrapper.
			READER_LOG (LOGTYPE_INFO, "Shutdown command in received. Going to shutdown...\n");
			this->m_shutdown = 1;
		} else if (pCmd->getMessageID() == 979700) {
			// If the received message ID is 979700, then set the number of tags per antenna to 0.
			READER_LOG (LOGTYPE_INFO, "Tag remove command in received. Removing all tags from FOV...\n");
			this->m_physicalReader->m_NumberOfStubTagsPerAntenna = 0;
			boost::thread pThread1(boost::bind(&StubReaderApp::clearVirtualTagOfReader, this->m_physicalReader->m_StubApp));
		} else if (pCmd->getMessageID() == 979701) {
			// If the received message ID is 979701, then set the number of tags per antenna to 2.
			READER_LOG (LOGTYPE_INFO, "Tag 2 add command in received. Adding 2 tags to FOV...\n");
			this->m_physicalReader->m_NumberOfStubTagsPerAntenna = 2;
			//this->_pReader->_pStubApp->clearVirtualTagOfReader();
			boost::thread pThread1(boost::bind(&StubReaderApp::clearVirtualTagOfReader, this->m_physicalReader->m_StubApp));
			boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
			//this->_pReader->_pStubApp->addRandomVirtualTags(2);
			boost::thread pThread2(boost::bind(&StubReaderApp::addRandomVirtualTags, this->m_physicalReader->m_StubApp, 2));
		} else if (pCmd->getMessageID() > 1000000 && pCmd->getMessageID() < 1000099) {
			// If the received message ID is bigger than 1000000, it is GPI event.
			// For example, in 10000$%, $ is port number and % is event.
			int msgID = pCmd->getMessageID();
			int GPIPortNum = (msgID / 10) % 10;
			int GPIEvent = msgID % 10;
			this->m_physicalReader->m_StubApp->invokeGPIEvent(GPIPortNum, GPIEvent);
		} else {
			// This is normal sequence of CLOSE_CONNECTION. Close the connection.
			this->m_receivedCloseConnection = 1;
		}
#else
		this->m_receivedCloseConnection = 1;
#endif
	}

	LLRP::CLLRPStatus *pRspStatus = new LLRP::CLLRPStatus();
	pRspStatus->setStatusCode(pResult.first);
	pRspStatus->setErrorDescription(pResult.second);
	pRsp->setLLRPStatus(pRspStatus);

	//For transactional message, the message ID shall be the same
	pRsp->setMessageID(pCmd->getMessageID());

	READER_LOG(LOGTYPE_INFO, "CLOSE_CONNECTION is finished\n");

	return 0;
}

int ELFIN::LLRPCore::connectReaderApp(StubReaderApp* __ReaderApp) {
	__ReaderApp->_isEmulator = 0;
	int result = this->m_physicalReader->connectStubApp(__ReaderApp);
	if (result == 0) {
		__ReaderApp->_isConnectedToReader = 1;
	}
	return result;
}

int ELFIN::LLRPCore::setReaderConfig(LLRP::CMessage* pMessage) {
	LLRP::CMessage **pRsp = new LLRP::CMessage*;
	int result = this->handleSetReaderConfig(pMessage, pRsp);
	delete *pRsp; // delete the response message
	delete pRsp;
	return result;
}

/**
 * @param pLogLevel Log level. (e.g. LOGTYPE_ALL)
 * @remark Log level is as follows,\n
 * ALL > TRACE > DEBUG > INFO > WARN > ERROR > FATAL > OFF
 */
void ELFIN::LLRPCore::setLogLevel(enum ELogType pLogLevel) {
	Utils::setLogLevel(pLogLevel);
}

int ELFIN::LLRPCore::loadDefaultConfigFile() {
	return this->setReaderConfig(this->m_LLRPConfig->_pDefaultConfig);
}

int ELFIN::LLRPCore::sendMessage(LLRP::CMessage *__pMessage) {
	return this->m_connMgr->sendMessage(__pMessage);
}

int ELFIN::LLRPCore::isConnected() {
	return this->m_connMgr->isConnected();
}

ELFIN::Result_t ELFIN::LLRPCore::getErrorResult() {
	switch (m_ErrorDetails.m_eResultCode) {
	case LLRP::RC_OK:
		return Result_t(LLRP::StatusCode_M_Success, Utils::getUTF8Str(""));
	case LLRP::RC_BadVersion:
		return Result_t(LLRP::StatusCode_M_UnsupportedVersion, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));
	case LLRP::RC_FieldOverrun:
	case LLRP::RC_FieldUnderrun:
		return Result_t(LLRP::StatusCode_M_FieldError, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));
	case LLRP::RC_MissingParameter:
		return Result_t(LLRP::StatusCode_M_MissingParameter, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));
	case LLRP::RC_UnexpectedParameter:
		return Result_t(LLRP::StatusCode_M_UnexpectedParameter, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));
	case LLRP::RC_UnknownMessageType:
		return Result_t(LLRP::StatusCode_M_UnsupportedMessage, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));
	case LLRP::RC_UnknownParameterType:
		return Result_t(LLRP::StatusCode_M_UnknownParameter, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));

	case LLRP::RC_Botch:
	case LLRP::RC_EnrollBadTypeNumber:
	case LLRP::RC_InvalidChoiceMember:
	case LLRP::RC_InvalidLength:
	case LLRP::RC_MiscError:
	case LLRP::RC_ReservedBitsOverrun:
	case LLRP::RC_ReservedBitsUnderrun:
	case LLRP::RC_UnalignedBitField:
	case LLRP::RC_UnalignedReservedBits:
		return Result_t(LLRP::StatusCode_A_Invalid, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));

	case LLRP::RC_ExcessiveLength:
	case LLRP::RC_ExtraBytes:
		return Result_t(LLRP::StatusCode_A_OutOfRange, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));

	case LLRP::RC_FieldAllocationFailed:
	case LLRP::RC_MessageAllocationFailed:
	case LLRP::RC_ParameterAllocationFailed:
	case LLRP::RC_NotAllowedAtExtensionPoint:
	case LLRP::RC_MissingResponseType:
	case LLRP::RC_RecvBufferOverflow:
	case LLRP::RC_RecvFramingError:
	default:
		// These are not message errors
		return Result_t(LLRP::StatusCode_R_DeviceError, Utils::getUTF8Str(m_ErrorDetails.m_pWhatStr));
	}
}
