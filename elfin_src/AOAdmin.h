/**
 * @file    AOAdmin.h
 * @brief
 */

#ifndef __LLRP_READER__AOADMIN_H__
#define __LLRP_READER__AOADMIN_H__


namespace ELFIN
{
	class LLRPCore;
	class AccessOperation;
	class Scheduler;
	class AOAdmin;
}

namespace ELFIN
{
	/** @class AOAdmin
	 * @brief AOAdmin(Access Operation Admin) class manages states of AccessSpecs.
	 */
	class AOAdmin
	{
		public:
			/// Constructor of AOAdmin class
			AOAdmin (LLRPCore *__pLLRPCore);
			/// Destructor of AOAdmin class
			~AOAdmin();
			/// Enable the AccessSpec. If the given AccessSpec ID is 0, enable all AccessSpec.
			int enableAccessSpec(LLRP::llrp_u32_t aAccessSpecID);
			/// Add the given AccessSpec
			int addAccessSpec(LLRP::CAccessSpec *aAccessSpec);
			/// Disable the AccessSpec. If the given AccessSpec ID is 0, disable all AccessSpec.
			int disableAccessSpec(LLRP::llrp_u32_t aAccessSpecID);
			/// Delete the AccessSpec. If the given AccessSpec ID is 0, delete all AccessSpec.
			int deleteAccessSpec(LLRP::llrp_u32_t aAccessSpecID);
			/// Store the pointer of all AccessSpecs to vector, and return it.
			std::vector<LLRP::CAccessSpec*> *getAccessSpecs();

			/**@{*/
			/// Iterator access method
			ELFIN::AOMap::iterator beginAOMap();
			ELFIN::AOMap::iterator endAOMap();
			int countAOMap();
			/**@}*/
		private:
			ELFIN::LLRPCore* _pLLRPCore;
			/// CAccessSpec container
			CAOMap _CAOMap;
			/// Access Operation container
			AOMap _AOMap;
	};
}

#endif /* __LLRP_READER__AOADMIN_H__ */
