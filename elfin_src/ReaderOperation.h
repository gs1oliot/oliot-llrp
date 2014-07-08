/**
 * @file    ReaderOperation.h
 * @brief
 */

#ifndef __LLRP_READER__READEROPERATION_H__
#define __LLRP_READER__READEROPERATION_H__

#include <vector>
#include <boost/thread/thread.hpp>
#include "AbstractAntennaOperation.h"
#include "CConnectionFnCMgr.h"
#include "ELFIN_Platform.h"

namespace ELFIN
{
class AbstractAntennaOperation;
class StubReader;
class LLRPCore;
class CConnectionFnCMgr;
class ReaderOperation;
class ROAdmin;
class AOAdmin;
}

namespace ELFIN
{
/** @class ReaderOperation
 * @brief A single reader operation generated based on the LLRP ROSpec message.
 */
class ReaderOperation: public ELFIN::TimerTask
{
public:
	/// Constructor of ReaderOperation class
	ReaderOperation(LLRP::CROSpec *aROSpec, LLRPCore *__pLLRPCore);
	/// Destructor of ReaderOperation class
	~ReaderOperation();
	/// Creates the thread of ReaderOperation using runRO()
	void run();
	/// This method is used to make ReaderOperation thread.
	void runRO();
	/// Send stop command to this ReaderOperation and wait until it actually stops.
	void stopRO();
	/// This method is used to make thread for stop trigger  which is related to time. After given time, interrupts tag singulation.
	void run_ROSpecStopTriggerThread(int stopMS);
	/// Static method to stop and delete given ReaderOperation
	static void deleteRO(ReaderOperation *pRO);
	/// Schedule this ReaderOperation to the Scheduler
	void schedule();
	/// Build RO_ACCESS_REPORT message with currently accumulated TagReportData.
	LLRP::CRO_ACCESS_REPORT *getCurrentReport(TagReportSet *pTagReportSet, TagReportSet *pUpperTagReportSet, int tagCount);
	/// When the start trigger of ROSpec is invoked, the scheduled ROSpec executes this. This method performs the main operation of ReaderOperation.
	void handle_timeout(const boost::system::error_code& e);
	/// This method is called right before the ROSpec stops. But this is not used currently.
	void handle_timeout_finalize(const boost::system::error_code& e);

	ELFIN::Scheduler* _pScheduler;
private:
	AbsAntennaOpsMap _absAntennaOps;
	StubReader *_pReader;
	boost::asio::io_service _ios;
public:
	LLRP::CROSpec *_CROSpec;
	LLRP::CROReportSpec *_pROReportSpec;
	LLRPCore *_pLLRPCore;
	LLRPReaderConfig *_pConfig;
	ROAdmin *_pROAdmin;
	boost::thread _pThread;
	boost::thread _pROSpecStopTriggerThread;
	int _pROSpecStopTriggerInvoked;
	int _pLoopSpecInfinity;
	int _pStopping;
	LLRP::llrp_u32_t _pLoopSpecLoopCount;
	LLRP::CRO_ACCESS_REPORT *_pPendingReport;
};
}

#endif /* __LLRP_READER__READEROPERATION_H__ */
