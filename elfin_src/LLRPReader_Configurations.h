/**
 * @file    LLRPReader_Configurations.h
 * @brief
 */

#ifndef __LLRP_READER__LLRPREADER_CONFIGURATIONS_H__
#define __LLRP_READER__LLRPREADER_CONFIGURATIONS_H__

namespace ELFIN {

/** @class LLRPReaderConfig
 *  @brief Stores default configurations which can be defined with SET_READER_CONFIG message.\n
 *  Also, it parses the contents of default config file (\a config.llrp) and stores it as a default configuration.
 */
class LLRPReaderConfig {
public:
	/// Constructor of LLRPReaderConfig class
	LLRPReaderConfig(const char *__pConfigFileName);
	/// Destructor of LLRPReaderConfig class
	~LLRPReaderConfig();
	/// Initialize the object by setting default specs, loading default config file, and build config map
	int init();
	/// Clear all default specs
	int clear();
	/// Clear and re-initialize this object
	int resetToFactoryDefault();
	/// Change LLRPConfigurationStateValue randomly. If this value is changed, we can assume that some configuration values have changed.
	void invalidateStateValue();
	/// Load, parse, and store the default config file (\a config.llrp)
	int loadDefaultConfigFile();

private:
	/// Set the default specs
	int setDefaultSpecs();
	/// Build ReaderEventNotification configuration map for EventNotifier
	int buildConfigMaps();

public:
	char *_pConfigFileName;

	/* Configurations */
	LLRP::CROReportSpec *_pDefaultROReportSpec;
	LLRP::CAccessReportSpec *_pDefaultAccessReportSpec;
	LLRP::CKeepaliveSpec *_pDefaultKeepaliveSpec;
	LLRP::CEventsAndReports *_pDefaultEventsAndReports;
	LLRP::CReaderEventNotificationSpec *_pDefaultReaderEventNotificationSpec;
	/// This value is changed randomly, when some configuration value is changed. If this value is changed, we can assume that some configuration values have changed.
	int _pLLRPConfigurationStateValue;
	/// ReaderEventNotification configuration map. EventNotifier refers this map to determine which event should be sent.
	int _pEventNotiArray[EVENT_NOTI_KIND_COUNT];

	/// Default SET_READER_CONFIG message object, which is loaded from default config file (\a config.llrp)
	LLRP::CMessage *_pDefaultConfig;
};
}


#endif /* LLRPREADER_CONFIGURATIONS_H_ */
