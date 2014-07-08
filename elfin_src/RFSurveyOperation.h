/**
 * @file    RFSurveyOperation.h
 * @brief
 */

#ifndef __LLRP_READER__RFSURVEYOPERATION_H__
#define __LLRP_READER__RFSURVEYOPERATION_H__

// #include "CRFSurveySpec.h"
#include "AbstractAntennaOperation.h"
#include "ELFIN_Platform.h"

namespace ELFIN
{
class StubReader;
class StubAntenna;
class CRFSurveySpec;
class RFSurveyOperation;
}

namespace ELFIN
{
/** @class RFSurveyOperation
 * @brief A single RF Survey operation generated based on the RFSurveySpec parameter in the given ROSpec.
 * @remark Because RFSurveyOperation is optional feature, this is not implemented.
 */
class RFSurveyOperation: public ELFIN::AbstractAntennaOperation
{
public:
	RFSurveyOperation(CRFSurveySpec *pSpec, StubReader *__pReader, ReaderOperation *__pRO, int __pSpecIndex);
	~RFSurveyOperation() {};
	CRFSurveySpec* _CRFSurveySpec;
	/// @fixme  This should be modified to return RFSurveyReport. Currently, AbstractAntennaOperation has prototype as follows, so cannot modify.
	TagReportSet *run();
private:
};
}

#endif /* __LLRP_READER__RFSURVEYOPERATION_H__ */
