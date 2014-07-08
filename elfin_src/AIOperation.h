/**
 * @file    AIOperation.h
 * @brief
 */

#ifndef __LLRP_READER__AIOPERATION_H__
#define __LLRP_READER__AIOPERATION_H__

namespace ELFIN
{
class StubReader;
class StubAntenna;
class AbstractAntennaOperation;
class AIOperation;
class AOAdmin;
}

namespace ELFIN
{
/** @class AIOperation
 * @brief A single antenna inventory operation generated based on the AISpec parameter in the given ROSpec.
 */
class AIOperation: public ELFIN::AbstractAntennaOperation
{
public:
	/// Constructor of AIOperation class
	AIOperation (LLRP::CROSpec *__pROSpec, LLRP::CAISpec *__pAISpec,
			StubReader *__pReader, ReaderOperation *__pRO, AOAdmin *__pAOAdmin, int __pSpecIndex);
	/// Destructor of AIOperation class
	~AIOperation();

	/// Add the given pRpt to the pSet. If the same EPC number exists in the vector, then overwrite it.
	static int addTagReportUnique(TagReportSet *pSet, LLRP::CTagReportData *pRpt);

	/// Run the AIOperation according to the CAISpec
	TagReportSet *run();

private:
	LLRP::CROSpec* _pROSpec;
	LLRP::CAISpec* _pAISpec;
	LLRP::CROReportSpec* _pROReportSpec;
	LLRP::CC1G2EPCMemorySelector* _pMemSelector;
	/// CTagReportData container
	TagReportSet *_pTagReportSet;
	boost::thread _pAISpecStopTriggerThread;
	int isOnSingulation;
	AOAdmin *_pAOAdmin;

	/// Create base tag report data for the given StubTag.
	LLRP::CTagReportData *createTagReportData(StubTag *pStubTag, int pInvSpecID);
	/// This method is used to make thread for stop trigger which is related to time. After given time, interrupts tag singulation.
	void run_AISpecStopTriggerThread(int stopMS);
};

}

#endif /* __LLRP_READER__AIOPERATION_H__ */
