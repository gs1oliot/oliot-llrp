/**
 * @file    ELFIN_ProgOpts.h
 * @brief
 */

#ifndef __LLRP_READER__PROGRAMOPTION_H__
#define __LLRP_READER__PROGRAMOPTION_H__

#include <exception>
#include <boost/program_options.hpp>
#include "ELFIN_Defs.h"

namespace ELFIN
{
/** @class ProgramOptions
 * @brief Parse the configuration file and store it.
 */
class ProgramOptions
{
public: 
	/// Constructor of ProgramOptions class
	ProgramOptions();
	/// Destructor of ProgramOptions class
	virtual ~ProgramOptions(){};

public:
	/// Parse the command line and configuration file options, and store them
	int processCmdlineOptions(int argc, char *argv []);

	bool isHelpOption();

	void printHelp();

	/// Print current option values
	void printOptions();
	/**@{*/
	/// Returns the corresponding option value
	int getReaderInitiatedOpt();
	std::string getFnCAddress();
	int getFnCPort();
	std::string getEPC();
	int getIsEmulatorMode();
	int getGPIPortCount();
	int getGPOPortCount();
	int getAntennaCount();
	int getReadCycle();
	int getVirtualTagCount();
	enum ELogType getLogLevel();
	std::string getLLRPFileName();
	/**@}*/


private:
	boost::program_options::variables_map _vm;
	boost::program_options::options_description _visibleDesc;
};
}

#endif//__LLRP_READER__PROGRAMOPTION_H__
