/**
 * @file    EventNotifier.h
 * @brief
 */

#include <exception>

#ifndef __LLRP_READER__EVENTNOTIFIER_H__
#define __LLRP_READER__EVENTNOTIFIER_H__
#include "LLRPCore.h"
#include "ELFIN_Platform.h"

namespace ELFIN
{
class EventNotifier;
class LLRPReaderConfig;
class CConnectionFnCMgr;
}

namespace ELFIN
{
/** @class EventNotifier
 * @brief Create and send ReaderEventNotifications to the FnC server.
 * @remark Event methods of this class can be just called where the event should be sent,
 * because each method checks whether the corresponding event is enabled or not before sending it.
 */
class EventNotifier
{
public:
	/// Constructor of EventNotifier class
	EventNotifier(LLRPCore *__pLLRPCore);
	/// Destructor of EventNotifier class
	~EventNotifier();
	/**@{*/
	/// Send corresponding event, if that event notification is enabled.
	int sendGPIEvent(LLRP::llrp_u32_t GPIPortNumber, LLRP::llrp_u32_t GPIEvent);
	int sendROSpecEvent(LLRP::llrp_u32_t ROSpecID, enum LLRP::EROSpecEventType EventType,
			LLRP::llrp_u32_t PreemtingROSpecID);
	int sendAISpecEvent(LLRP::llrp_u32_t ROSpecID, LLRP::llrp_u16_t SpecIndex,
			enum LLRP::EAISpecEventType EventType);
	int sendConnectionAttemptEvent(enum LLRP::EConnectionAttemptStatusType EventType);
	int sednConnectionCloseEvent();
	int sendSpecLoopEvent(LLRP::llrp_u32_t ROSpecID, LLRP::llrp_u32_t LoopCount);
	/**@}*/
	/**@{*/
	/// These events are not implemented, because they are optional.
	int sendHoppingEvent(int HopTableID, int NextChannelIndex);
	int sendReportBufferLevelWarningEvent(int ReportBufferPercentageFull);
	int sendReportBufferOverflowErrorEvent();
	int sendReaderExceptionEvent();
	int sendRFSurveyEvent(int ROSpecID, int SpecIndex,
			enum LLRP::ERFSurveyEventType EventType);
	int sendAntennaEvent(int AntennaID, enum LLRP::EAntennaEventType EventType);
	/**@}*/

private:
	LLRPCore *_pLLRPCore;
	LLRPReaderConfig *_pConfig;
};
}

#endif /* __LLRP_READER__EVENTNOTIFIER_H__ */
