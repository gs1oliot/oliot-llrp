/**
 * @file    StubApp.h
 * @brief
 */

#ifndef __LLRP_READER_STUBAPP_H__
#define __LLRP_READER_STUBAPP_H__

#include <boost/function.hpp>
//#include "StubReader.h"
#include "StubTag.h"
#include "../ELFIN_Defs.h"

namespace ELFIN
{
class StubReader;
class StubReaderApp;
}

namespace ELFIN
{
/** @class StubReaderApp
 * @brief Reader application which starts and controls LLRP Wrapper. LLRP Wrapper calls callbacks of this object.\n
 * Developer of LLRP Wrapper should inherit this class to make reader application.
 */
class StubReaderApp
{
public:
	/// Constructor of StubReaderApp class. This is usually called by inherited reader applications.
	StubReaderApp();
	/// Constructor of StubReaderApp class. This is usually called by StubReader.
	StubReaderApp(StubReader *__physicalReader);
	/// Destructor of StubReaderApp class
	virtual ~StubReaderApp();

	/// StubReader registers StubReader::invokeGPIEventCallback to StubReaderApp with this method. This callback is called in invokeGPIEvent().
	//int regGPICallback(boost::function<int(StubReader*, LLRP::llrp_u16_t, enum LLRP::EGPIPortState)> callback);
	int regGPICallback(boost::function<int(StubReader*, uint16_t, int)> callback);
	/// This callback is invoked when the LLRP Wrapper is successfully connected to F&C server and ready to work.
	virtual void initializaionFinishedCallback();
	/// This callback is invoked when the ROSpec with GPI start trigger finished its operation and sent RO_ACCESS_REPORT.
	virtual void GPITriggeredInventoryFinishedCallback(int pROSpecID, TagVector *pReportedTagVector);
	/// This callback is invoked when the ROSpec with non-GPI start trigger finished its operation and sent RO_ACCESS_REPORT.
	virtual void InventoryOperationFinishedCallback(int pROSpecID, TagVector *pReportedTagVector);
	/// This callback is invoked when the LLRP Wrapper is successfully connected to F&C server.
	virtual void notifyConnectionResultCallback(int connResult);
	/// This callback is invoked when the LLRP Wrapper is initialized and ready to start connection with F&C server.
	virtual void readyToConnectCallback();
	/// This callback is invoked when the connection to F&C server is closed and LLRP Wrapper is re-initialized.
	virtual void connectionClosedCallback();
	/// This callback is invoked when the SET_READER_CONFIG message ordered to change GPO port value.
	virtual void GPOEventCallback(int portNum, int state);

	/// Remove all virtual tags of the LLRP Wrapper
	void clearVirtualTagOfReader();
	/// Add a virtual tag to the LLRP Wrapper with given parameter.
	ELFIN::StubTag *addVirtualTagToReader(int pAntennaID, const char *EPCID);
	/// Remove a virtual tag of the LLRP Wrapper which has same EPCID with the given parameter
	void removeVirtualTagFromReader(const char *EPCID);
	/// Add virtual tags with randomly generated EPCID. If there is 4 antennas and parameter is 3, 12 different virtual tags are added.
	void addRandomVirtualTags(int numOfTagsPerAntenna);

	/// When GPI event is invoked, call this method to send it to registered callback.
	int invokeGPIEvent(unsigned short int GPIPortNum, int state);

	StubReader *_pReader;
	//boost::function<int(StubReader*, LLRP::llrp_u16_t, enum LLRP::EGPIPortState)> _GPICallBack;
	boost::function<int(StubReader*, uint16_t, int)> _GPICallBack;
	int _isEmulator;
	int _isConnectedToReader;
};
}


#endif /* __LLRP_READER_STUBAPP_H__ */
