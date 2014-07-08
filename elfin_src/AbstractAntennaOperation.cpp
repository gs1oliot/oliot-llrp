/**
 * @file    AbstractAntennaOperation.cpp
 * @brief
 */

#include "AbstractAntennaOperation.h"

ELFIN::AbstractAntennaOperation::AbstractAntennaOperation(StubReader *__pReader, ReaderOperation *__pRO,
		const LLRP::CTypeDescriptor *__pTypeDescriptor, int __pSpecIndex)
: _pReader(__pReader), _pRO(__pRO), _pTypeDescriptor(__pTypeDescriptor), _pSpecIndex(__pSpecIndex){

}
