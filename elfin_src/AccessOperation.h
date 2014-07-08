/**
 * @file    AccessOperation.h
 * @brief
 */

#ifndef __LLRP_READER__ACCESSOPERATION_H__
#define __LLRP_READER__ACCESSOPERATION_H__

#include "ELFIN_Platform.h"

namespace ELFIN
{
	class AOAdmin;
	class StubTag;
	class AbstractAntennaOperation;
	class AccessOperation;
	class LLRPCore;
}

namespace ELFIN
{
	/** @class AccessOperation
	 * @brief A single access operation generated based on the given AccessSpec parameter.
	 */
	class AccessOperation
	{
		public:
			/// Constructor of AccessOperation class
			AccessOperation(LLRPCore *__pLLRPCore, LLRP::CAccessSpec *__pCAccessSpec);
			/// Get the OpSpec id from the given OpSpec.
			static LLRP::llrp_u16_t getOpSpecID(LLRP::CAccessCommandOpSpec *pOpSpec);
			/// Get the OpSpec id from the given AccessCommandOpSpecResult
			static LLRP::llrp_u16_t getACOpSpecResultOPSpecID(LLRP::CAccessCommandOpSpecResult* pAccessCommandOpSpecResult);
			/// Compare two CAccessCommandOpSpecResult. If they are identical, returns 0. If not, returns non-zero value.
			static int compareACOpSpecResult(
					LLRP::CAccessCommandOpSpecResult* pResult1, LLRP::CAccessCommandOpSpecResult* pResult2);
			/// Check the validity of AccessOperation object.
			int isValid();

			/// Match the provided StubTag and ROSpecID to the current C1G2TargetTag
			int matchTagSpec (StubTag *pStubTag, LLRP::llrp_u32_t pROSpecID);
			/// Apply OpSpec to the provided StubTag. The OpSpecResult would be stored to the pTagReport object.
			int executeOpSpecs (StubTag *pStubTag, LLRP::CTagReportData *pTagReport);

			/// Handles C1G2Read OpSpec. The result is stored to the pTagReport object.
			int handleC1G2Read(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag *pStubTag, LLRP::CTagReportData *pTagReport);
			/// Handles C1G2Write OpSpec. The result is stored to the pTagReport object.
			int handleC1G2Write(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag *pStubTag, LLRP::CTagReportData *pTagReport);
			/**@{*/
			/// Handles corresponding OpSpec. But this method is not implemented yet.
			int handleC1G2Kill(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag *pStubTag, LLRP::CTagReportData *pTagReport);
			int handleC1G2Lock(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag *pStubTag, LLRP::CTagReportData *pTagReport);
			int handleC1G2BlockErase(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag *pStubTag, LLRP::CTagReportData *pTagReport);
			int handleC1G2BlockWrite(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag *pStubTag, LLRP::CTagReportData *pTagReport);
			/**@}*/

			LLRPCore *_pLLRPCore;
			LLRP::CAccessSpec* _pCAccessSpec;
			LLRP::CAccessReportSpec *_pAccessReportSpec;
			LLRP::CC1G2TagSpec* _pTagSpec;
			LLRP::llrp_u16_t _pOperationCount;

		private:
			int _valid;
			/// CAccessCommandOpSpec container
			OPSpecsMap _pOpSpecMap;
	};
}

#endif /* __LLRP_READER__ACCESSOPERATION_H__ */
