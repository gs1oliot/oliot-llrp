/**
 * @file    GPIEventListener.h
 * @brief
 */

#ifndef __LLRP_READER__GPIEVENTLISTENER_H__
#define __LLRP_READER__GPIEVENTLISTENER_H__
#include "boost/tuple/tuple.hpp"
#include <boost/thread/mutex.hpp>

namespace ELFIN
{
class GPIEventListener;
}

namespace ELFIN
{
/** @class GPIEventListener
 * @brief Interface for handling the signal from the GPI port of LLRP Reader.
 */
class GPIEventListener
{
public:
	/// Constructor of GPIEventListener class
	GPIEventListener();
	/// Destructor of GPIEventListener class
	virtual ~GPIEventListener();
	/// Perform the operation with given port number of GPI port and its state
	virtual void onGPIEvent(LLRP::llrp_u16_t GPIPortNum, enum LLRP::EGPIPortState GPIEvent) = 0;
	/// Register GPI event to _GPIEventRegistry
	int regGPIEvent(int GPIPortNum, int GPIEvent, int ROSpecID, enum EGPITriggerType trigType);
	/// Unregister GPI event from _GPIEventRegistry
	int unregGPIEvent(int GPIPortNum, int GPIEvent);
	GPIEventRegistry _GPIEventRegistry;
	boost::mutex _pGPIRegLock;
};
}

#endif /* __LLRP_READER__GPIEVENTLISTENER_H__ */
