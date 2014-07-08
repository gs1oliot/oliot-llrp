/**
 * @file    LLRPCore.h
 * @brief
 */

#ifndef __LLRP_READER__LLRPCore_H__
#define __LLRP_READER__LLRPCore_H__

#include <boost/thread/mutex.hpp>
#include "CConnectionFnCMgr.h"
#include "ROAdmin.h"
#include "ELFIN_ProgOpts.h"
#include "ELFIN_Platform.h"

namespace ELFIN
{
	class AOAdmin;
	class AbstractAntenna;
	class ROAdmin;
	class CConnectionFnCMgr;
	class StubReader;
	class LLRPReaderConfig;
	class StubReaderApp;
	class EventNotifier;

/**
 * @class  LLRPCore
 * @brief  LLRP Core Operation Component
 */
	class LLRPCore
	{
		public:
			/// Constructor of LLRPCore class
			LLRPCore(ProgramOptions* __progOpt);
			/// Destructor of LLRPCore class
			~LLRPCore();
			/// Initialize LLRPCore object by setting default values and instantiating all required objects.
			int init();
			/// Destroy all required objects and return to the initial state.
			int clear();
			/// Prepare connection to the FnC server. Initiate connection or wait for connection, according to the \a readerInitiated option.
			int startConnection();
			/// Start LLRP Wrapper. StubApp should execute this method to run the LLRP Wrapper.
			int run();
			/// Set the configuration of LLRPReader with SET_READER_CONFIG message object.
			int setReaderConfig(LLRP::CMessage *pMessage);
			/// Load default SET_READER_CONFIG message from LLRPReaderConfig object and apply it to LLRP Wrapper.
			int loadDefaultConfigFile();

		private:
			/**@{*/
			/// LLRP message handler method
			int handleGetSupportedVersion(LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleSetProtocolVersion(LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleGetReaderCapabilities (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);

			int handleAddROSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleDeleteROSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleStartROSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleStopROSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleEnableROSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleDisableROSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleGetROSpecs (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);

			int handleAddAccessSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleDeleteAccessSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleEnableAccessSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleDisableAccessSpec (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleGetAccessSpecs (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);

			int handleGetReaderConfig (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			int handleSetReaderConfig (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);

			int handleCloseConnection (LLRP::CMessage *_pCmd, LLRP::CMessage **_pRsp);
			/**@}*/
			/// Create Result_t based on the m_ErrorDetails
			Result_t getErrorResult();
			LLRP::CErrorDetails m_ErrorDetails;


		public:
			ProgramOptions* m_progOpt;
			LLRPReaderConfig* m_LLRPConfig;
			StubReader* m_physicalReader;
			ROAdmin* m_ROAdmin;
			AOAdmin* m_AOAdmin;
			CConnectionFnCMgr* m_connMgr;
			EventNotifier *m_EventNotifier;
			boost::mutex m_initLock;
			int m_shutdown;
			int m_receivedCloseConnection;
			int m_justInited;

		public:
			/// Connect the StubReaderApp(or the class which inherits StubReaderApp) to StubReader
			int connectReaderApp(StubReaderApp *__pStubApp);
			/// Set the log level of LLRP Wrapper
			static void setLogLevel(enum ELogType pLogLevel);

		public:
			/// Proxy method of CConnectionFnCMgr::sendMessage
			int sendMessage(LLRP::CMessage *__pMessage);
			/// Proxy method of CConnectionFnCMgr::isConnected
			int isConnected();
	};
}

#endif /* __LLRP_READER__LLRPCore_H__ */
