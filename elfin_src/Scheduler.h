/**
 * @file    Scheduler.h
 * @brief
 */


#ifndef __LLRP_READER__SCHEDULER_H__
#define __LLRP_READER__SCHEDULER_H__

#include "TimerTask.h"

namespace ELFIN
{
class TimerTask;
class Scheduler;
}

namespace ELFIN
{
/** @class Scheduler
 *  @brief TimerTask is scheduled to this class. It can be scheduled for one-time or repeated execution by a Timer.
 */
class Scheduler{

public:
	/// Constructor of Scheduler class
	Scheduler(boost::asio::io_service& ios):_strand(ios), _pdltimer(NULL)
	{
		//NOTE: strand must be instantiated sometime after ios has been instantiated.
		//_pStrand = new boost::asio::strand(_ios);
	}
	/// Destructor of Scheduler class
	virtual ~Scheduler(){
	}

	void run();

public:
	/**@{*/
	/// Schedule the given TimerTask with given options
	void schedule(TimerTask* task, long delay, unsigned long count, unsigned long offset);
	void schedule(TimerTask* task, long delay, unsigned long count);
	void schedule(TimerTask* task, long delay);
	/**@}*/
	boost::asio::io_service& get_io_service()
	{
		return _strand.get_io_service();
	}
private:
	/// When the scheduled TimerTask starts, this method is called to decide whether to re-enable the TimerTask for next execution.
	void _handle_timeout(TimerTask* pTmrTask, const boost::system::error_code& e);
private:

	/*
		Strands: Use Threads Without Explicit Locking
	 *  For more detail, http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/overview/core/strands.html
	 */
	boost::asio::strand _strand;
	boost::asio::deadline_timer *_pdltimer;
};

}


#endif /* __LLRP_READER__SCHEDULER_H__ */
