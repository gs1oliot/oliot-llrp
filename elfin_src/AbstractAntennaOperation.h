/**
 * @file    AbstractAntennaOperation.h
 * @brief
 */

#include <exception>
#ifndef __LLRP_READER__ABSTRACTANTENNAOPERATION_H__
#define __LLRP_READER__ABSTRACTANTENNAOPERATION_H__

#include <boost/unordered_set.hpp>
#include "Stubs/StubReader.h"
#include "TimerTask.h"
#include "Scheduler.h"
#include "ELFIN_Platform.h"

namespace ELFIN
{
class StubReader;
class StubAntenna;
class ReaderOperation;
class AbstractAntennaOperation;
}

namespace ELFIN
{
/** @class AbstractAntennaOperation
 * @brief Abstraction for the classes which perform several operation with the antennas and tags.
 */
class AbstractAntennaOperation
{
public:
	/// Constructor of AbstractAntennaOperation class
	AbstractAntennaOperation(StubReader *__pReader, ReaderOperation *__pRO,
			const LLRP::CTypeDescriptor *pTypeDescriptor, int __pSpecIndex);
	/// Destructor of AbstractAntennaOperation class
	virtual ~AbstractAntennaOperation() {};
	StubReader *_pReader;
	/// ReaderOperation which contains this operation
	ReaderOperation *_pRO;
	/// Type descriptor of this operation. This is used to distinguish the type of this operation.
	const LLRP::CTypeDescriptor *_pTypeDescriptor;
	int _pSpecIndex;

	/// Performs the operation
	/// @fixme  This should be fixed to conform RFSurveyOperation.
	virtual TagReportSet *run() = 0;
};
}

#endif /* __LLRP_READER__ABSTRACTANTENNAOPERATION_H__ */
