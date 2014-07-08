/**
 * @file    ROAdmin.h
 * @brief
 */

#ifndef __LLRP_READER__ROADMIN_H__
#define __LLRP_READER__ROADMIN_H__

#include <boost/thread/recursive_mutex.hpp>
#include "ELFIN_Platform.h"
#include "GPIEventListener.h"

namespace ELFIN
{
	class LLRPCore;
	class ReaderOperation;
	class Scheduler;
	class GPIEventListener;
	class ROAdmin;
}

namespace ELFIN
{
	/** @class ROAdmin
	 * @brief ROAdmin(Reader Operation Admin) class manages states and operations of ROSpecs and schedules them.
	 */
	class ROAdmin: public ELFIN::GPIEventListener
	{
		public:
			/// Constructor of ROAdmin class
			ROAdmin(ELFIN::LLRPCore* __LLRPCore);
			/// Destructor of ROAdmin class
			~ROAdmin();
			/// Enable the ROSpec. If the given ROSpec ID is 0, enable all ROSpec.
			int enableROSpec(LLRP::llrp_u32_t aROSpecID);
			/// Start the ROspec. If the given ROSpec ID is 0, it is illegal and returns error.
			int startROSpec(LLRP::llrp_u32_t aROSpecID);
			/// Stop the ROspec. If the given ROSpec ID is 0, it is illegal and returns error.
			int stopROSpec(LLRP::llrp_u32_t aROSpecID);
			/// Disable the ROSpec. If the given ROSpec ID is 0, disable all ROSpec.
			int disableROSpec(LLRP::llrp_u32_t aROSpecID);
			/// Add the given ROSpec
			int addROSpec(LLRP::CROSpec *aRoSpec);
			/// Delete the ROSpec. If the given ROSpec ID is 0, delete all ROSpec.
			int deleteROSpec(LLRP::llrp_u32_t aROSpecID);
			/// Store the pointer of all AccessSpecs to vector, and return it.
			std::vector<LLRP::CROSpec*> *getROSpecs();

			void onGPIEvent(LLRP::llrp_u16_t GPIPortNum, enum LLRP::EGPIPortState GPIEvent);
			int regGPIEvents(ReaderOperation *ro);
			int unregGPIEvents(ReaderOperation *ro);

			boost::recursive_mutex _pROLock;

		private:
			LLRPCore *_LLRPCore;
			// CROSpec container
			CROMap _CROMap;
			// Reader Operation container
			ROMap _ROMap;
	};
}

#endif /* __LLRP_READER__ROADMIN_H__ */
