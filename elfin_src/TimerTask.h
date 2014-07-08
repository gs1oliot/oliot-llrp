/**
 * @file    TimerTask.h
 * @brief
 */

#ifndef __LLRP_READER__TIMERTASK_h__
#define __LLRP_READER__TIMERTASK_h__

#include <boost/asio.hpp>
#include "ELFIN_Platform.h"
#include "Scheduler.h"

namespace ELFIN
{
class TimerTask;
}

namespace ELFIN
{
//Forward declaration
class Scheduler;


/** @class TimerTask
 * @brief Interface for operations with time-related boundary specs. For example, ReaderOperation extends TimerTask to perform periodic operation.
 */
class TimerTask
{
	friend class Scheduler;
public:
	//! MAX age of a timer task
	enum{
		COUNT_INFINITY = -1,
	};
	/// Constructor of TimerTask class
	TimerTask():_periodic(true), _active(false), _count(COUNT_INFINITY), _period(0), _pDTmr(NULL){}
	/// Destructor of TimerTask class
	virtual ~TimerTask()
	{
		READER_LOG (LOGTYPE_DEBUG, "Destroying TimerTask...\n");
		this->cancel();
	}

public:
	/// Initialize the TimerTask
	void initTimerTask()
	{
		this->cancel();
		_periodic = true;
		_active = false;
		_count = COUNT_INFINITY;
		_period = 0;
		_pDTmr = NULL;
	}
	/// Immediately let this TimerTask wake from blocking and be stopped.
	void cancel()
	{
		_active=false;
		if (_pDTmr != NULL) {
			_pDTmr->cancel();

			delete _pDTmr;
			_pDTmr = NULL;
		}
	}
	/// When the scheduled TimerTask starts, this method is called
	virtual void handle_timeout(const boost::system::error_code& e) = 0;
	/// Right after the final execution of TimerTask, this method is called. But it is not used currently.
	virtual void handle_timeout_finalize (const boost::system::error_code& e) = 0;


	long scheduledExecutionTime();

	bool is_periodic()
	{
		return _periodic;
	}

	unsigned long get_period()
	{
		return _period;
	}

	unsigned long get_count()
	{
		return _count;
	}
private:
	//! private member functions.

	void set_active(bool state)
	{
		_active = state;
	}
	void set_period(unsigned long p)
	{
		_period = p;
	}
	/*!
	set count to limite its lifetime
	 */
	void set_count(unsigned long count)
	{
		_count = count;
	}

	/*!
		decrease age and deactivate task if age expires.
	 */
	void decreaseCount()
	{
		if(_count != COUNT_INFINITY)
		{
			--_count;
			if(_count == 0)
			{
				_periodic = false;
				cancel();
			}
		}
	}

private:

	//! A public variable.

	bool _periodic;
	bool _active;
	int _count;
	unsigned long _period;
	/*!
      Deadline_timer which must exist per timer task basis.
	 */
public:
	boost::asio::deadline_timer *_pDTmr;

};

}
#endif /* __LLRP_READER__TIMERTASK_h__ */
