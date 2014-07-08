/**
 * @file    RFSurveyOperation.cpp
 * @brief
 */

#include "RFSurveyOperation.h"
#include "AbstractAntennaOperation.h"

ELFIN::RFSurveyOperation::RFSurveyOperation(CRFSurveySpec* pSpec, StubReader *__pReader, ReaderOperation *__pRO, int __pSpecIndex)
: ELFIN::AbstractAntennaOperation(__pReader, __pRO, &LLRP::CRFSurveySpec::s_typeDescriptor, __pSpecIndex) {
	_CRFSurveySpec = pSpec;
}

ELFIN::TagReportSet *ELFIN::RFSurveyOperation::run() {
	return NULL;
}
