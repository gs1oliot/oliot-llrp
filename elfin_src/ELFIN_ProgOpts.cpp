/**
 * @file    ELFIN_ProgOpts.cpp
 * @brief
 */

#include <exception>


#include <boost/program_options.hpp>
namespace progopt = boost::program_options;
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <iterator>

#include "ELFIN_ProgOpts.h"
#include "ELFIN_Platform.h"


namespace ELFIN{

//write your configuration here
static const char *programDescriptions = 
		"Usage: LLRPReader [--t] [] target_name \n"
		"@target_name: host name or ip address of a llrp server (e.g., fc middleware)"
		;

static const char *pDefaultconfig_filename = "reader.cfg";
static const char *pDefaultllrpconfig_filename = "config.llrp";
static const char *pDefaultconfig_name = "R905";
static const char *pDefaultconfig_reader_initiated = "false";
static const char *pDefaultconfig_fnc_address = "127.0.0.1";
static const char *pDefaultconfig_fnc_port = "5084";
static const char *pDefaultconfig_epc = "350011223344556677889900";
static const char *pDefaultconfig_emulator_mode = "true";
static const char *pDefaultconfig_gpiport_count = "2";
static const char *pDefaultconfig_gpoport_count = "2";
static const char *pDefaultconfig_antenna_count = "4";
static const char *pDefaultconfig_read_cycle = "5";
static const char *pDefaultconfig_virtual_tag_count = "0";
static const char *pDefaultconfig_log_level = "ALL";

//Constructor
ProgramOptions::ProgramOptions():_visibleDesc(programDescriptions)
{

}

int ProgramOptions::processCmdlineOptions(int argc, char *argv [])
{
	std::string opt_config_file;
	std::string opt_llrpconfig_file;
	std::string opt_name;
	std::string opt_epc;
	std::string opt_reader_initiated;
	std::string opt_fnc_address;
	std::string opt_fnc_port;
	std::string opt_emulator_mode;
	std::string opt_gpiport_count;
	std::string opt_gpoport_count;
	std::string opt_antenna_count;
	std::string opt_read_cycle;
	std::string opt_virtual_tag_count;
	std::string opt_log_level;

	/***********************************************************
		The parameters are option name, information about value, and option description. 
		describe general options
	 */
	boost::program_options::options_description generalOptionsDesc; //description about general options
	generalOptionsDesc.add_options()
        				("help", "produce help message")
        				("config,c", progopt::value<std::string>(&opt_config_file)->default_value(pDefaultconfig_filename),
        						"name of a file of a configuration.")
						("llrpconfig_file,l", progopt::value<std::string>(&opt_llrpconfig_file)->default_value(pDefaultllrpconfig_filename),
								"name of a file of a LLRP configuration.")
        						//Add general options here
        						;

	/************************************************************/
	// Declare a group of options that will be 
	// allowed both on command line and in
	// config file
	progopt::options_description config("Configuration"); //description about group
	config.add_options()
						("name,n", progopt::value<std::string>(&opt_name)->default_value(pDefaultconfig_name),
								"set a logical reader name")
        				("epc", progopt::value<std::string>(&opt_epc)->default_value(pDefaultconfig_epc),
        						"EPC ID of the reader.")
        				("reader_initiated", progopt::value<std::string>(&opt_reader_initiated)->default_value(pDefaultconfig_reader_initiated),
        						"set whether the reader runs as server or client")
						("fnc_address", progopt::value<std::string>(&opt_fnc_address)->default_value(pDefaultconfig_fnc_address),
								"set an address of fnc server")
						("fnc_port", progopt::value<std::string>(&opt_fnc_port)->default_value(pDefaultconfig_fnc_port),
								"set a port of fnc server")
						("emulator_mode", progopt::value<std::string>(&opt_emulator_mode)->default_value(pDefaultconfig_emulator_mode),
								"run the LLRPReader in emulator mode")
						("gpiport_count", progopt::value<std::string>(&opt_gpiport_count)->default_value(pDefaultconfig_gpiport_count),
								"number of GPI ports")
						("gpoport_count", progopt::value<std::string>(&opt_gpoport_count)->default_value(pDefaultconfig_gpoport_count),
								"number of GPO ports")
						("antenna_count", progopt::value<std::string>(&opt_antenna_count)->default_value(pDefaultconfig_antenna_count),
								"number of antennas")
						("read_cycle", progopt::value<std::string>(&opt_read_cycle)->default_value(pDefaultconfig_read_cycle),
								"Cycles to read per each tag read operation")
						("virtual_tag_count", progopt::value<std::string>(&opt_virtual_tag_count)->default_value(pDefaultconfig_virtual_tag_count),
								"Number of virtual tags to add to each antenna")
						("log_level", progopt::value<std::string>(&opt_log_level)->default_value(pDefaultconfig_log_level),
								"Log level to display (ALL > TRACE > DEBUG > INFO > WARN > ERROR > FATAL > OFF)")

								//Add configuration options here
								;


	// From the above configurations, the following enables the options above to be
	// shared by command line options as well as file options
	progopt::options_description cmdline_options;
	cmdline_options.add(generalOptionsDesc).add(config);


	//
	progopt::options_description config_file_options;
	config_file_options.add(config);

	//options that should be visible with help options
	_visibleDesc.add(generalOptionsDesc).add(config);

	//printOptions();

	/**
	That class is intended to store values of options,
	and can store values of arbitrary types. 
	Next, the calls to store, parse_command_line and notify 
	functions cause vm to contain all the options found on the command line.
	 */

	progopt::store(progopt::parse_command_line(argc, argv, _visibleDesc), _vm);
	progopt::notify(_vm);

	std::ifstream ifs(opt_config_file.c_str());
	if (!ifs)
	{
		std::cout << "can not open config file: " << opt_config_file << "\n";
		return -1;
	}
	else
	{
		store(parse_config_file(ifs, config_file_options), _vm);
		notify(_vm);
	}

	return 0;
}

void ProgramOptions::printOptions()
{
	std::cout << "Configuration options" << "\n";

	if (_vm.count("name")) {
		READER_LOG(LOGTYPE_INFO, "The logical reader name was set to '%s'.\n", _vm["name"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The logical reader name was not set.\n");
	}
	if (_vm.count("epc")) {
		READER_LOG(LOGTYPE_INFO, "The EPCID of the reader was set to '%s'.\n", _vm["epc"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The EPCID of the reader was not set.\n");
	}
	if (_vm.count("reader_initiated")) {
		READER_LOG(LOGTYPE_INFO, "The reader_initiated option was set to '%s'.\n", _vm["reader_initiated"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The reader_initiated option was not set.\n");
	}
	if (_vm.count("fnc_address")) {
		READER_LOG(LOGTYPE_INFO, "The address of FnC middleware was set to '%s'.\n", _vm["fnc_address"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The address of FnC middleware was not set.\n");
	}
	if (_vm.count("fnc_port")) {
		READER_LOG(LOGTYPE_INFO, "The port number to use was set to '%s'.\n", _vm["fnc_port"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The port number to use was not set.\n");
	}
	if (_vm.count("emulator_mode")) {
		READER_LOG(LOGTYPE_INFO, "The emulator_mode option was set to '%s'.\n", _vm["emulator_mode"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The emulator_mode was not set.\n");
	}
	if (_vm.count("gpiport_count")) {
		READER_LOG(LOGTYPE_INFO, "The gpiport_count option was set to '%s'.\n", _vm["gpiport_count"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The gpiport_count was not set.\n");
	}
	if (_vm.count("gpoport_count")) {
		READER_LOG(LOGTYPE_INFO, "The gpoport_count option was set to '%s'.\n", _vm["gpoport_count"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The gpoport_count was not set.\n");
	}
	if (_vm.count("antenna_count")) {
		READER_LOG(LOGTYPE_INFO, "The antenna_count option was set to '%s'.\n", _vm["antenna_count"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The antenna_count was not set.\n");
	}
	if (_vm.count("read_cycle")) {
		READER_LOG(LOGTYPE_INFO, "The read_cycle option was set to '%s'.\n", _vm["read_cycle"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The read_cycle was not set.\n");
	}
	if (_vm.count("virtual_tag_count")) {
		READER_LOG(LOGTYPE_INFO, "The virtual_tag_count option was set to '%s'.\n", _vm["virtual_tag_count"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The virtual_tag_count was not set.\n");
	}
	if (_vm.count("llrpconfig_file")) {
		READER_LOG(LOGTYPE_INFO, "The llrpconfig_file option was set to '%s'.\n", _vm["llrpconfig_file"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The llrpconfig_file was not set.\n");
	}
	if (_vm.count("log_level")) {
		READER_LOG(LOGTYPE_INFO, "The log_level option was set to '%s'.\n", _vm["log_level"].as<std::string>().c_str());
	} else {
		READER_LOG(LOGTYPE_INFO, "The log_level was not set.\n");
	}

}

int ProgramOptions::getReaderInitiatedOpt() {
	int result;
	try {
	if (_vm.count("reader_initiated")) {
		const std::string optVal = _vm["reader_initiated"].as<std::string>();
		if (strcmp(optVal.c_str(), "true") == 0) {
			//READER_LOG(LOGTYPE_TRACE, "Reader initiated connection was set to %s.\n", optVal.c_str());
			result = 1;
		}
		else if (strcmp(optVal.c_str(), "false") == 0) {
			//READER_LOG(LOGTYPE_TRACE, "Reader initiated connection was set to %s.\n", optVal.c_str());
			result = 0;
		}
		else {
			READER_LOG(LOGTYPE_ERROR, "Reader initiated connection value is in wrong format: %s.\n", optVal.c_str());
			result = -1;
		}
	} else {
		READER_LOG(LOGTYPE_TRACE, "Reader initiated connection was not set. Default is false.\n");
		result = 0;
	}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return result;
}

std::string ProgramOptions::getFnCAddress() {
	try {
		if (_vm.count("fnc_address")) {
			std::string optVal = _vm["fnc_address"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "FnC address was set to %s.\n", optVal.c_str());
			return optVal;
		} else {
			READER_LOG(LOGTYPE_TRACE, "FnC address was not set.\n");
			return NULL;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return NULL;
	}
}

int ProgramOptions::getFnCPort() {
	int portNum = -1;
	try {
		if (_vm.count("fnc_port")) {
			std::string optVal = _vm["fnc_port"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "FnC port was set to %s.\n", optVal.c_str());
			//std::istringstream (optVal) >> portNum;
			portNum = boost::lexical_cast<int>(optVal.c_str());
			return portNum;
		} else {
			READER_LOG(LOGTYPE_TRACE, "FnC port was not set.\n");
			return -1;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
}

std::string ProgramOptions::getEPC() {
	try {
		if (_vm.count("epc")) {
			std::string optVal = _vm["epc"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "EPC was set to %s.\n", optVal.c_str());
			return optVal;
		} else {
			READER_LOG(LOGTYPE_TRACE, "EPC was not set.\n");
			return NULL;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return NULL;
	}
}

int ProgramOptions::getIsEmulatorMode() {
	int result;
	try {
		if (_vm.count("emulator_mode")) {
			const std::string optVal = _vm["emulator_mode"].as<std::string>();
			if (strcmp(optVal.c_str(), "true") == 0) {
				//READER_LOG(LOGTYPE_TRACE, "emulator_mode was set to %s.\n", optVal.c_str());
				result = 1;
			}
			else if (strcmp(optVal.c_str(), "false") == 0) {
				//READER_LOG(LOGTYPE_TRACE, "emulator_mode was set to %s.\n", optVal.c_str());
				result = 0;
			}
			else {
				READER_LOG(LOGTYPE_ERROR, "emulator_mode is in wrong format: %s.\n", optVal.c_str());
				result = -1;
			}
		} else {
			READER_LOG(LOGTYPE_TRACE, "Reader initiated connection was not set. Default is false.\n");
			result = 0;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return result;
}

int ProgramOptions::getGPIPortCount() {
	int gpiport_count = -1;
	try {
		if (_vm.count("fnc_port")) {
			std::string optVal = _vm["gpiport_count"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "gpiport_count was set to %s.\n", optVal.c_str());
			gpiport_count = boost::lexical_cast<int>(optVal.c_str());
			return gpiport_count;
		} else {
			READER_LOG(LOGTYPE_TRACE, "gpiport_count port was not set.\n");
			return -1;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
}

int ProgramOptions::getGPOPortCount() {
	int gpoport_count = -1;
	try {
		if (_vm.count("gpoport_count")) {
			std::string optVal = _vm["gpoport_count"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "gpoport_count was set to %s.\n", optVal.c_str());
			gpoport_count = boost::lexical_cast<int>(optVal.c_str());
			return gpoport_count;
		} else {
			READER_LOG(LOGTYPE_TRACE, "gpoport_count port was not set.\n");
			return -1;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
}



int ProgramOptions::getAntennaCount() {
	int antenna_count = -1;
	try {
		if (_vm.count("antenna_count")) {
			std::string optVal = _vm["antenna_count"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "antenna_count was set to %s.\n", optVal.c_str());
			antenna_count = boost::lexical_cast<int>(optVal.c_str());
			return antenna_count;
		} else {
			READER_LOG(LOGTYPE_TRACE, "antenna_count port was not set.\n");
			return -1;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
}

int ProgramOptions::getReadCycle() {
	int read_cycle = 5;
	try {
		if (_vm.count("read_cycle")) {
			std::string optVal = _vm["read_cycle"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "read_cycle was set to %s.\n", optVal.c_str());
			read_cycle = boost::lexical_cast<int>(optVal.c_str());
			return read_cycle;
		} else {
			READER_LOG(LOGTYPE_TRACE, "read_cycle port was not set.\n");
			return 5;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return 5;
	}
}

int ProgramOptions::getVirtualTagCount() {
	int virtual_tag_count = 5;
	try {
		if (_vm.count("virtual_tag_count")) {
			std::string optVal = _vm["virtual_tag_count"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "virtual_tag_count was set to %s.\n", optVal.c_str());
			virtual_tag_count = boost::lexical_cast<int>(optVal.c_str());
			return virtual_tag_count;
		} else {
			READER_LOG(LOGTYPE_TRACE, "virtual_tag_count port was not set.\n");
			return 2;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return 2;
	}
}

std::string ProgramOptions::getLLRPFileName() {
	try {
		if (_vm.count("llrpconfig_file")) {
			std::string optVal = _vm["llrpconfig_file"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "llrpconfig_file was set to %s.\n", optVal.c_str());
			return optVal;
		} else {
			READER_LOG(LOGTYPE_TRACE, "llrpconfig_file was not set.\n");
			return NULL;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return NULL;
	}
}

enum ELFIN::ELogType ProgramOptions::getLogLevel() {
	try {
		if (_vm.count("log_level")) {
			std::string optVal = _vm["log_level"].as<std::string>();
			//READER_LOG(LOGTYPE_TRACE, "log_level was set to %s.\n", optVal.c_str());
			// ALL > TRACE > DEBUG > INFO > WARN > ERROR > FATAL > OFF
			switch (optVal.c_str()[0]) {
			case 'A':
			case 'a':
				return LOGTYPE_ALL;
			case 'T':
			case 't':
				return LOGTYPE_TRACE;
			case 'D':
			case 'd':
				return LOGTYPE_DEBUG;
			case 'I':
			case 'i':
				return LOGTYPE_INFO;
			case 'W':
			case 'w':
				return LOGTYPE_WARN;
			case 'E':
			case 'e':
				return LOGTYPE_ERROR;
			case 'F':
			case 'f':
				return LOGTYPE_FATAL;
			case 'O':
			case 'o':
				return LOGTYPE_OFF;
			}
			return LOGTYPE_ALL;
		} else {
			READER_LOG(LOGTYPE_TRACE, "llrpconfig_file was not set.\n");
			return LOGTYPE_ALL;
		}
	} catch ( const std::exception& e ) {
		std::cerr << e.what() << std::endl;
		return LOGTYPE_ALL;
	}
}


bool ProgramOptions::isHelpOption()
{
	return (_vm.count("help") == 1 ? true : false);
}

void ProgramOptions::printHelp()
{
	if (_vm.count("help")) {
		std::cout << _visibleDesc << "\n";
	}
}

}
