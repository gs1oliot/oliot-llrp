/**
 * @file    AccessOperation.cpp
 * @brief
 */

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>

#include "LLRPCore.h"
#include "Stubs/StubTag.h"
#include "Stubs/StubReader.h"
#include "AccessOperation.h"
#include "ELFIN_Platform.h"

/**
 * @warning This constructor may create invalid object. You should check its validity with isValid() method.
 */
ELFIN::AccessOperation::AccessOperation(LLRPCore *__pLLRPCore, LLRP::CAccessSpec *__pCAccessSpec)
: _pLLRPCore(__pLLRPCore), _pCAccessSpec(__pCAccessSpec), _pOperationCount(0), _valid(TRUE) {
	if (_pCAccessSpec->getProtocolID() == LLRP::AirProtocols_EPCGlobalClass1Gen2) {
		if (_pCAccessSpec->getAccessReportSpec() != NULL) {
			_pAccessReportSpec = _pCAccessSpec->getAccessReportSpec();
		}
		else {
			_pAccessReportSpec = __pLLRPCore->m_LLRPConfig->_pDefaultAccessReportSpec;
		}

		_pTagSpec = (LLRP::CC1G2TagSpec *) _pCAccessSpec->getAccessCommand()->getAirProtocolTagSpec();
		// Use biggestOpSpecID to confirm that OpSpecID is in sequence starts with 1
		LLRP::llrp_u16_t biggestOpSpecID = 0;
		for (std::list<LLRP::CParameter *>::iterator iter = _pCAccessSpec->getAccessCommand()->beginAccessCommandOpSpec();
				iter != _pCAccessSpec->getAccessCommand()->endAccessCommandOpSpec();iter++) {
			LLRP::CAccessCommandOpSpec *pOpSpec = (LLRP::CAccessCommandOpSpec *) (*iter);
			LLRP::llrp_u16_t pOpSpecID = getOpSpecID(pOpSpec);
			biggestOpSpecID = (biggestOpSpecID > pOpSpecID) ? biggestOpSpecID : pOpSpecID;
			if (pOpSpecID == 0) {
				READER_LOG(LOGTYPE_ERROR, "AccessOperation: OpSpec ID cannot be 0.\n");
				_valid = FALSE;
				break;
			}
			if (_pOpSpecMap[pOpSpecID] != NULL) {
				READER_LOG(LOGTYPE_ERROR, "AccessOperation: Duplicate OpSpec ID %d.\n", pOpSpecID);
				_valid = FALSE;
				break;
			}
			_pOpSpecMap[pOpSpecID] = pOpSpec;
		}
		if (_pOpSpecMap.size() != biggestOpSpecID) {
			READER_LOG(LOGTYPE_ERROR, "AccessOperation: OpSpecID is not in sequence.\n");
			_valid = FALSE;
		}
	}
	else {
		throw "not implemented";
	}
}

/**
 * @remark Because LLRP::CAccessCommandOpSpec type does not have method to get the OpSpec id,
 * this method casts the OpSpec to its own type and return the OpSpec id.
 */
LLRP::llrp_u16_t ELFIN::AccessOperation::getOpSpecID(LLRP::CAccessCommandOpSpec* pOpSpec) {
	LLRP::llrp_u16_t pOpSpecID = 0;
	// Need to cast CAccessCommandOpSpec to CElement to get the type descriptor.
	LLRP::CElement *pElement = (LLRP::CElement *)pOpSpec;
	if (&LLRP::CC1G2Read::s_typeDescriptor == pElement->m_pType) {
		LLRP::CC1G2Read *pSpecificOpSpec = (LLRP::CC1G2Read *)pOpSpec;
		pOpSpecID = pSpecificOpSpec->getOpSpecID();
	} else if (&LLRP::CC1G2Write::s_typeDescriptor == pElement->m_pType) {
		LLRP::CC1G2Write *pSpecificOpSpec = (LLRP::CC1G2Write *)pOpSpec;
		pOpSpecID = pSpecificOpSpec->getOpSpecID();
	} else if (&LLRP::CC1G2Kill::s_typeDescriptor == pElement->m_pType) {
		LLRP::CC1G2Kill *pSpecificOpSpec = (LLRP::CC1G2Kill *)pOpSpec;
		pOpSpecID = pSpecificOpSpec->getOpSpecID();
	} else if (&LLRP::CC1G2Lock::s_typeDescriptor == pElement->m_pType) {
		LLRP::CC1G2Lock *pSpecificOpSpec = (LLRP::CC1G2Lock *)pOpSpec;
		pOpSpecID = pSpecificOpSpec->getOpSpecID();
	} else if (&LLRP::CC1G2BlockErase::s_typeDescriptor == pElement->m_pType) {
		LLRP::CC1G2BlockErase *pSpecificOpSpec = (LLRP::CC1G2BlockErase *)pOpSpec;
		pOpSpecID = pSpecificOpSpec->getOpSpecID();
	} else if (&LLRP::CC1G2BlockWrite::s_typeDescriptor == pElement->m_pType) {
		LLRP::CC1G2BlockWrite *pSpecificOpSpec = (LLRP::CC1G2BlockWrite *)pOpSpec;
		pOpSpecID = pSpecificOpSpec->getOpSpecID();
	}
	else {
		READER_LOG(LOGTYPE_ERROR, "OpSpec %s is not implemented.\n", pElement->m_pType->m_pName);
	}

	return pOpSpecID;
}

int ELFIN::AccessOperation::matchTagSpec(StubTag* pStubTag, LLRP::llrp_u32_t pROSpecID) {
	if (_pTagSpec->countC1G2TargetTag() > 1) {
		READER_LOG (LOGTYPE_ERROR, "matchTagSpec: Matching two TagSpecs is not supported\n");
		return FALSE; // Matching two TagSpecs is optional, so it is not supported yet.
	}
	for (std::list<LLRP::CC1G2TargetTag *>::iterator iter = _pTagSpec->beginC1G2TargetTag();
			iter != _pTagSpec->endC1G2TargetTag();iter++) {
		LLRP::CC1G2TargetTag *targetTag = (*iter);
		// If the length is 0, then this TagSpec matches all tags
		if (targetTag->getTagData().m_nBit == 0) {
			return TRUE;
		}
		/// @fixme  need to remove redundant code
		if (this->_pLLRPCore->m_progOpt->getIsEmulatorMode() == 1) {
			MemoryBank *targetMB = pStubTag->_MB.at(targetTag->getMB());
			// Compare the masked memory bank contents and the match data, 8 bits at once.
			/// @fixme  Need to modify not only support the pointer multiple of 8, but also other values
			if ((targetTag->getPointer() % 8 != 0) || (targetTag->getTagMask().m_nBit % 8 != 0)) {
				READER_LOG (LOGTYPE_INFO, "matchTagSpec: Matching part of byte is not supported\n");
				return FALSE; // Matching part of byte is not supported
			}

			unsigned int offsetMBSlotNum = targetTag->getPointer() / 8;
			unsigned int tagMaskSlotCount = targetTag->getTagMask().m_nBit / 8;

			LLRP::llrp_u1_t match = TRUE;

			for (unsigned int i = 0;i < tagMaskSlotCount;i++) {
				if (offsetMBSlotNum + i >= targetMB->size()) {
					READER_LOG(LOGTYPE_INFO, "matchTagSpec: pointer + mask overflows the size of memory bank %d.\n", targetTag->getMB());
					return FALSE;
				}
				//printf ("DEBUG::: targetMB->at(offset(%d) + i(%d)), m_pValue[i]=%d\n", offset, i, targetTag->getTagMask().m_pValue[i]);
				LLRP::llrp_u8_t maskedMBContents = targetMB->at(offsetMBSlotNum + i) & targetTag->getTagMask().m_pValue[i]; //error???
				LLRP::llrp_u8_t maskedData = targetTag->getTagData().m_pValue[i] & targetTag->getTagMask().m_pValue[i];
				if (maskedMBContents != maskedData) {
					match = FALSE;
					break;
				}
			}
			if (targetTag->getMatch() == match) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
		else if (this->_pLLRPCore->m_progOpt->getIsEmulatorMode() == 0) {
			/// @todo  Optimize this, by not reading whole mb contents, but just target area.
			// If the reading of tag memory bank fails, return false.
			if (this->_pLLRPCore->m_physicalReader->readTagMemoryBank(0, pStubTag, targetTag->getMB(), 0, 0) != 0)
				return FALSE;

			MemoryBank *targetMB = pStubTag->_MB.at(targetTag->getMB());
			// Compare the masked memory bank contents and the match data, 8 bits at once.
			/// @fixme  Need to modify not only support the pointer multiple of 8, but also other values
			if ((targetTag->getPointer() % 8 != 0) || (targetTag->getTagMask().m_nBit % 8 != 0)) {
				READER_LOG (LOGTYPE_INFO, "matchTagSpec: Matching part of byte is not supported\n");
				return FALSE; // Matching part of byte is not supported
			}

			unsigned int offsetMBSlotNum = targetTag->getPointer() / 8;
			unsigned int tagMaskSlotCount = targetTag->getTagMask().m_nBit / 8;

			LLRP::llrp_u1_t match = TRUE;

			for (unsigned int i = 0;i < tagMaskSlotCount;i++) {
				if (offsetMBSlotNum + i >= targetMB->size()) {
					READER_LOG(LOGTYPE_INFO, "matchTagSpec: pointer + mask overflows the size of memory bank %d.\n", targetTag->getMB());
					return FALSE;
				}
				//printf ("DEBUG::: targetMB->at(offset(%d) + i(%d)), m_pValue[i]=%d\n", offset, i, targetTag->getTagMask().m_pValue[i]);
				LLRP::llrp_u8_t maskedMBContents = targetMB->at(offsetMBSlotNum + i) & targetTag->getTagMask().m_pValue[i]; //error???
				LLRP::llrp_u8_t maskedData = targetTag->getTagData().m_pValue[i] & targetTag->getTagMask().m_pValue[i];
				if (maskedMBContents != maskedData) {
					match = FALSE;
					break;
				}
			}
			if (targetTag->getMatch() == match) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
	}
	return TRUE;
}

int ELFIN::AccessOperation::executeOpSpecs(StubTag* pStubTag, LLRP::CTagReportData *pTagReport) {
	if (_pCAccessSpec->getProtocolID() == LLRP::AirProtocols_EPCGlobalClass1Gen2) {
		for (std::list<LLRP::CParameter *>::iterator iter = _pCAccessSpec->getAccessCommand()->beginAccessCommandOpSpec();
				iter != _pCAccessSpec->getAccessCommand()->endAccessCommandOpSpec();iter++) {
			// Need to cast CAccessCommandOpSpec to CElement to get the type descriptor.
			LLRP::CElement *pElement = (LLRP::CElement *) (*iter);
			LLRP::CAccessCommandOpSpec *pOpSpec = (LLRP::CAccessCommandOpSpec *) (*iter);
			if (&LLRP::CC1G2Read::s_typeDescriptor == pElement->m_pType) {
				handleC1G2Read(pOpSpec, pStubTag, pTagReport);
			} else if (&LLRP::CC1G2Write::s_typeDescriptor == pElement->m_pType) {
				handleC1G2Write(pOpSpec, pStubTag, pTagReport);
			} else if (&LLRP::CC1G2Kill::s_typeDescriptor == pElement->m_pType) {
				handleC1G2Kill(pOpSpec, pStubTag, pTagReport);
			} else if (&LLRP::CC1G2Lock::s_typeDescriptor == pElement->m_pType) {
				handleC1G2Lock(pOpSpec, pStubTag, pTagReport);
			} else if (&LLRP::CC1G2BlockErase::s_typeDescriptor == pElement->m_pType) {
				handleC1G2BlockErase(pOpSpec, pStubTag, pTagReport);
			} else if (&LLRP::CC1G2BlockWrite::s_typeDescriptor == pElement->m_pType) {
				handleC1G2BlockWrite(pOpSpec, pStubTag, pTagReport);
			}
			else {
				READER_LOG(LOGTYPE_ERROR, "OpSpec %s is not implemented.\n", pElement->m_pType->m_pName);
			}
		}
	}
	else {
		throw "not implemented";
	}
	return 0;
}

int ELFIN::AccessOperation::handleC1G2Read(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag* pStubTag, LLRP::CTagReportData *pTagReport) {
	LLRP::CC1G2Read *pOper = (LLRP::CC1G2Read *)pOpSpec;

	READER_LOG (LOGTYPE_DEBUG, "OpSpecID: %d, MB: %d, PWD: %d, WordPtr: %d, WordCnt: %d\n", pOper->getOpSpecID(), pOper->getMB(), pOper->getAccessPassword(), pOper->getWordPointer(), pOper->getWordCount());

	LLRP::CC1G2ReadOpSpecResult *pResult = new LLRP::CC1G2ReadOpSpecResult();

	// If it is not emulator mode and reading tag memory bank failes, then send error report.
	if (this->_pLLRPCore->m_progOpt->getIsEmulatorMode() == 0) {
		if (this->_pLLRPCore->m_physicalReader->readTagMemoryBank(0, pStubTag, pOper->getMB(), 0, 0) != 0) {
			pResult->setResult(LLRP::C1G2ReadResultType_Nonspecific_Tag_Error);
			pTagReport->addAccessCommandOpSpecResult((LLRP::CParameter *) pResult);
			return -1;
		}
	}

	//MemoryBank *targetMB = pStubTag->_MB.at(pOper->getMB());
	MemoryBank *targetMB = pStubTag->_MB[pOper->getMB()];

	LLRP::llrp_u16_t pOperWordCount = pOper->getWordCount();
	pResult->setOpSpecID(pOper->getOpSpecID());
	if (pOperWordCount == 0) {
		pOperWordCount = targetMB->size() / 2;
		READER_LOG (LOGTYPE_DEBUG, "WordCount is changed from 0 to %d\n", pOperWordCount);
	}
	// WordPointer is 1-bit, Word is 16-bit, and MB is 8-bit per one space
	if (pOper->getWordPointer() / 8 + pOperWordCount * 2 > (int) targetMB->size()) {
		pResult->setResult(LLRP::C1G2ReadResultType_Tag_Memory_Overrun_Error);
	}
	if (pOper->getMB() == 1) {
		// If the target MB is EPC MB, then operation is different from others. (LLRP 1.1 Spec 16.2.1.3.2.1)
		LLRP::llrp_u16v_t readData(pOperWordCount);
		// Merge two MB space into one readData space
		for (int i = 0;i < pOperWordCount;i++) {
			int targetMemAddr = i * 2 + pOper->getWordPointer() / 8;
			readData.m_pValue[i] = targetMB->at(targetMemAddr) * 0x100 + targetMB->at(targetMemAddr + 1) * 0x1;
		}
		pResult->setReadData(readData);
		pResult->setResult(LLRP::C1G2ReadResultType_Success);
	}
	else {
		LLRP::llrp_u16v_t readData(pOperWordCount);
		// Merge two MB space into one readData space
		for (int i = 0;i < pOperWordCount;i++) {
			int targetMemAddr = i * 2 + pOper->getWordPointer() / 8;
			readData.m_pValue[i] = targetMB->at(targetMemAddr) * 0x100 + targetMB->at(targetMemAddr + 1) * 0x1;
		}
		pResult->setReadData(readData);
		pResult->setResult(LLRP::C1G2ReadResultType_Success);

	}
	pTagReport->addAccessCommandOpSpecResult((LLRP::CParameter *) pResult);
	return 0;
}

int ELFIN::AccessOperation::handleC1G2Write(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag* pStubTag, LLRP::CTagReportData *pTagReport) {
	LLRP::CC1G2Write *pOper = (LLRP::CC1G2Write *)pOpSpec;
	LLRP::CC1G2WriteOpSpecResult *pResult = new LLRP::CC1G2WriteOpSpecResult();
	int readResult = 0;
	READER_LOG (LOGTYPE_DEBUG, "OpSpecID: %d, MB: %d, PWD: %d, WordPtr: %d, WriteDataWords: %d\n",
			pOper->getOpSpecID(), pOper->getMB(), pOper->getAccessPassword(), pOper->getWordPointer(), pOper->getWriteData().m_nValue);

	pResult->setOpSpecID(pOper->getOpSpecID());
	if (this->_pLLRPCore->m_progOpt->getIsEmulatorMode() == 0) {
		readResult = this->_pLLRPCore->m_physicalReader->writeTagMemoryBank(0, pStubTag, pOper->getMB(), 0, pOper->getWriteData());
		if (readResult != 0) {
			pResult->setNumWordsWritten(0);
			// Because the real reader API does not support, setting as just any error.
			pResult->setResult(LLRP::C1G2WriteResultType_Nonspecific_Tag_Error);
		}
		else {
			pResult->setNumWordsWritten(pOper->getWriteData().m_nValue);
			pResult->setResult(LLRP::C1G2WriteResultType_Success);
		}
	}
	else {
		MemoryBank *targetMB = pStubTag->_MB[pOper->getMB()];
		LLRP::CC1G2WriteOpSpecResult *pResult = new LLRP::CC1G2WriteOpSpecResult();
		pResult->setOpSpecID(pOper->getOpSpecID());
		// WordPointer is 1-bit, Word is 16-bit, and MB is 8-bit per one space
		if (pOper->getWordPointer() / 8 + pOper->getWriteData().m_nValue > (int) targetMB->size()) {
			pResult->setResult(LLRP::C1G2WriteResultType_Tag_Memory_Overrun_Error);
			// llrp_u16v_t contains words, so divide it to two memory bank slot.
			for (int i = 0;i < pOper->getWriteData().m_nValue;i++) {
				int targetMemAddr = i * 2 + pOper->getWordPointer() / 8;
				targetMB->at(targetMemAddr) = pOper->getWriteData().m_pValue[i] / 0x100;
				targetMB->at(targetMemAddr + 1) = pOper->getWriteData().m_pValue[i] % 0x100;
				printf ("%04X ", pOper->getWriteData().m_pValue[i]);
			}
			printf ("\n");
			pResult->setNumWordsWritten(pOper->getWriteData().m_nValue);
			pResult->setResult(LLRP::C1G2WriteResultType_Success);
		}
	}
	pTagReport->addAccessCommandOpSpecResult((LLRP::CParameter *) pResult);
	return 0;
}

int ELFIN::AccessOperation::handleC1G2Kill(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag* pStubTag, LLRP::CTagReportData *pTagReport) {
	LLRP::CC1G2Kill *pOper = (LLRP::CC1G2Kill *)pOpSpec;
	pOper->getKillPassword();
	LLRP::CC1G2KillOpSpecResult *pResult = new LLRP::CC1G2KillOpSpecResult();

	pResult->setResult(LLRP::C1G2KillResultType_Nonspecific_Tag_Error);
	pTagReport->addAccessCommandOpSpecResult((LLRP::CParameter *) pResult);
	return 0;
}

int ELFIN::AccessOperation::handleC1G2Lock(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag* pStubTag, LLRP::CTagReportData *pTagReport) {
	LLRP::CC1G2Lock *pOper = (LLRP::CC1G2Lock *)pOpSpec;
	pOper->getAccessPassword();

	LLRP::CC1G2LockOpSpecResult *pResult = new LLRP::CC1G2LockOpSpecResult();

	pTagReport->addAccessCommandOpSpecResult((LLRP::CParameter *) pResult);
	return 0;
}

int ELFIN::AccessOperation::handleC1G2BlockErase(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag* pStubTag, LLRP::CTagReportData *pTagReport) {
	LLRP::CC1G2BlockErase *pOper = (LLRP::CC1G2BlockErase *)pOpSpec;
	pOper->getAccessPassword();

	LLRP::CC1G2BlockEraseOpSpecResult *pResult = new LLRP::CC1G2BlockEraseOpSpecResult();

	pTagReport->addAccessCommandOpSpecResult((LLRP::CParameter *) pResult);
	return 0;
}

int ELFIN::AccessOperation::handleC1G2BlockWrite(LLRP::CAccessCommandOpSpec *pOpSpec, StubTag* pStubTag, LLRP::CTagReportData *pTagReport) {
	LLRP::CC1G2BlockWrite *pOper = (LLRP::CC1G2BlockWrite *)pOpSpec;
	pOper->getAccessPassword();

	LLRP::CC1G2BlockWriteOpSpecResult *pResult = new LLRP::CC1G2BlockWriteOpSpecResult();

	pTagReport->addAccessCommandOpSpecResult((LLRP::CParameter *) pResult);
	return 0;
}

int ELFIN::AccessOperation::isValid() {
	return _valid;
}

/**
 * @remark Because LLRP::CAccessCommandOpSpecResult type does not have method to get the OpSpec id,
 * this method casts the OpSpecResult to its own type and return the OpSpec id.
 */
LLRP::llrp_u16_t ELFIN::AccessOperation::getACOpSpecResultOPSpecID(
		LLRP::CAccessCommandOpSpecResult* pAccessCommandOpSpecResult) {
	LLRP::CElement *pElement = (LLRP::CElement *) pAccessCommandOpSpecResult;
	if (&LLRP::CC1G2ReadOpSpecResult::s_typeDescriptor == pElement->m_pType) {
		return ((LLRP::CC1G2ReadOpSpecResult *) pAccessCommandOpSpecResult)->getOpSpecID();
	} else if (&LLRP::CC1G2WriteOpSpecResult::s_typeDescriptor == pElement->m_pType) {
		return ((LLRP::CC1G2WriteOpSpecResult *) pAccessCommandOpSpecResult)->getOpSpecID();
	} else if (&LLRP::CC1G2KillOpSpecResult::s_typeDescriptor == pElement->m_pType) {
		return ((LLRP::CC1G2KillOpSpecResult *) pAccessCommandOpSpecResult)->getOpSpecID();
	} else if (&LLRP::CC1G2LockOpSpecResult::s_typeDescriptor == pElement->m_pType) {
		return ((LLRP::CC1G2LockOpSpecResult *) pAccessCommandOpSpecResult)->getOpSpecID();
	} else if (&LLRP::CC1G2BlockEraseOpSpecResult::s_typeDescriptor == pElement->m_pType) {
		return ((LLRP::CC1G2BlockEraseOpSpecResult *) pAccessCommandOpSpecResult)->getOpSpecID();
	} else if (&LLRP::CC1G2BlockWriteOpSpecResult::s_typeDescriptor == pElement->m_pType) {
		return ((LLRP::CC1G2BlockWriteOpSpecResult *) pAccessCommandOpSpecResult)->getOpSpecID();
	} else {
		return -1;
	}
}


int ELFIN::AccessOperation::compareACOpSpecResult(
		LLRP::CAccessCommandOpSpecResult* pResult1, LLRP::CAccessCommandOpSpecResult* pResult2) {
	LLRP::CElement *pElement1 = (LLRP::CElement *) pResult1;
	LLRP::CElement *pElement2 = (LLRP::CElement *) pResult2;
	char *pBuf1 = new char[100*1024];
	char *pBuf2 = new char[100*1024];
	LLRP::toXMLString(pElement1, pBuf1, 100*1024);
	LLRP::toXMLString(pElement2, pBuf2, 100*1024);
	int cmpResult = strcmp(pBuf1, pBuf2);

	delete pBuf1;
	delete pBuf2;
	return cmpResult;
}
