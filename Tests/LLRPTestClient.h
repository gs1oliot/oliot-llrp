/*
 * TestClient.h
 *
 *  Created on: Aug 31, 2013
 *      Author: iot-team
 */

#ifndef TESTCLIENT_H_
#define TESTCLIENT_H_

#include "../LTK/LTKCPP/Library/ltkcpp.h"
using namespace LLRP;

class LLRPTestClient
{
public:
	enum ECLOSECONNECTION_CUSTOM_COMMAND {
		CLOSECONNECTION_CUSTOM_COMMAND_SHUTDOWN = 979797,
		CLOSECONNECTION_CUSTOM_COMMAND_PLACE_0_TAGS = 979700,
		CLOSECONNECTION_CUSTOM_COMMAND_PLACE_2_TAGS = 979701
	};

	int 						_shutdown;

	/** Verbose level, incremented by each -v on command line */
	int                         m_Verbose;

	/** ModeIndex requested [012345], -1 means no mode given
	 ** and use the default */
	int                         m_ModeIndex;
	/** Quiet flag -q, don't print the tag reports */
	int                         m_Quiet;
	/** Count of tag reports (RO_ACCESS_REPORT) */
	int                         m_nTagReport;

	const char *				m_ReaderHostName;

	/** Connection to the LLRP reader */
	CConnection *               m_pConnectionToReader;


	CTypeRegistry *             pTypeRegistry;

	LLRPTestClient (const char *pReaderHostName);

	int stop ();

	int
	openConnectionToReader (void);

	int
	waitconenctionFromReader (void);

	int
	checkConnectionStatus (void);

	int
	scrubConfiguration (void);

	int
	resetConfigurationToFactoryDefaults (void);

	int
	deleteAllROSpecs (void);

	int
	deleteAllAccessSpecs (void);

	int
	prepareReader(void);

	int
	getAllCapabilities (void);

	int
	getAllConfiguration (void);

	int
	configureNotificationStates (void);

	int
	addROSpec (void);

	int
	enableROSpec (llrp_u32_t ROSpecID, const char *testName);

	int
	startAndMonitorOperation (void);

	void
	printTagReportData (
			CRO_ACCESS_REPORT *       pRO_ACCESS_REPORT);

	void
	printOneTagReportData (
			CTagReportData *          pTagReportData);

	int
	checkLLRPStatus (
			CLLRPStatus *             pLLRPStatus,
			const char *                    pWhatStr);

	CMessage *
	transact (
			CMessage *                pSendMsg);

	CMessage *
	recvMessage (
			int                       nMaxMS);

	int
	sendMessage (
			CMessage *                pSendMsg);

	void
	printXMLMessage (
			CMessage *                pMessage);

	LLRP::CAISpec*
	getBasicAISpec();

	LLRP::CROSpec*
	getBasicROSpec();

	LLRP::CAccessSpec*
	getBasicAccessSpec();

	LLRP::CC1G2TagSpec*
	getTagSpecMatchAll();

	LLRP::CC1G2Write*
	getOpSpecWriteEPC();

	LLRP::CC1G2Read*
	getOpSpecReadEPC();

	LLRP::CInventoryParameterSpec*
	getROSpecFilterMatchAll();


	int
	checkLLRPConfigurationStateValue(llrp_u32_t *pLLRPConfigurationStateValue, const char *testName);

	int
	startROSpec(llrp_u32_t ROSpecID, const char *testName);

	int
	stopROSpec(llrp_u32_t ROSpecID, const char *testName);

	int
	deleteROSpec(llrp_u32_t ROSpecID, const char *testName);

	int
	disableROSpec(llrp_u32_t ROSpecID, const char *testName);

	int
	enableAccessSpec(llrp_u32_t AccessSpecID, const char *testName);

	int
	disableAccessSpec(llrp_u32_t AccessSpecID, const char *testName);

	int
	deleteAccessSpec(llrp_u32_t AccessSpecID, const char *testName);

	int
	sendCloseConnectionWithCustomCommand(enum ECLOSECONNECTION_CUSTOM_COMMAND pCommand);

	int
	sendCloseConnectionWithCustomCommand(int GPIPortNum, int Event);

	CREADER_EVENT_NOTIFICATION *
	recvNotification(int nMaxMs, const char *testName);

	CRO_ACCESS_REPORT *
	getROAccessReport(int nMaxMs, const char *testName);

	uint64_t getCurrentTimeMilliseconds();

	std::size_t getTagReportHash(LLRP::CTagReportData *pData);

	int compareCElems(LLRP::CElement *pElem1, LLRP::CElement *pElem2);
};

#endif /* TESTCLIENT_H_ */
