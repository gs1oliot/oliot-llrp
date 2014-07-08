/**
 * @file    Scheduler.cpp
 * @brief
 */

#include <exception>
#include <vector>
#include <string>
#include <set>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>

#include "Scheduler.h"
/*
void ELFIN::Scheduler::schedule(ELFIN::TimerTask aTask) {
	throw "Not yet implemented";
}
 */


void ELFIN::Scheduler::_handle_timeout(TimerTask* pTmrTask, const boost::system::error_code& e)
{
	assert(pTmrTask);

	if(pTmrTask->_active == true)
	{
		pTmrTask->handle_timeout(e);

		pTmrTask->decreaseCount();

		if(pTmrTask->_active == true && pTmrTask->is_periodic())
		{
			boost::asio::deadline_timer &tmr = *pTmrTask->_pDTmr;

			boost::posix_time::milliseconds delay(pTmrTask->get_period());

			tmr.expires_at(tmr.expires_at() + delay);
			//boost::asio::deadline_timer timer(_ios, boost::posix_time::seconds(delay));

			tmr.async_wait(_strand.wrap(boost::bind(&Scheduler::_handle_timeout, this, pTmrTask, boost::asio::placeholders::error)));

		}
		else
		{
			pTmrTask->initTimerTask();
			//pTmrTask->handle_timeout_finalize(e);
		}

	}

}

void ELFIN::Scheduler::schedule(TimerTask* pTmrTask, long delay, unsigned long count)
{
	schedule(pTmrTask, delay, count, 0);
}

void ELFIN::Scheduler::schedule(TimerTask* pTmrTask, long delay, unsigned long count, unsigned long offset)
{
#if 0
	boost::posix_time::milliseconds Offsetwait(offset);

	boost::posix_time::milliseconds Twait(delay);
	pTmrTask->set_period(delay);

	pTmrTask->_pDTmr = new boost::asio::deadline_timer(get_io_service(),
			Twait/*this parameter must be set for async op.*/);

	assert(pTmrTask->_pDTmr);

	boost::asio::deadline_timer &tmr = *pTmrTask->_pDTmr;
	//if(pTmrTask->_pdTmr)
	//	throw exception();

	pTmrTask->set_active(true);
	pTmrTask->set_count(count);


	if(offset > 0)
		tmr.expires_at(tmr.expires_at() + Offsetwait);

	if(delay < 0)
		tmr.expires_at(tmr.expires_at() + Twait);
	tmr.async_wait(_strand.wrap(boost::bind(&Scheduler::_handle_timeout, this, pTmrTask, boost::asio::placeholders::error)));

#else
	boost::posix_time::milliseconds Offsetwait(offset);

	boost::posix_time::milliseconds Twait(0);
	pTmrTask->set_period(delay);

	pTmrTask->_pDTmr = new boost::asio::deadline_timer(get_io_service(),
			Twait/*this parameter must be set for async op.*/);

	assert(pTmrTask->_pDTmr);

	boost::asio::deadline_timer &tmr = *pTmrTask->_pDTmr;
	//if(pTmrTask->_pdTmr)
	//	throw exception();

	pTmrTask->set_active(true);
	pTmrTask->set_count(count);


	if(offset > 0)
		tmr.expires_at(tmr.expires_at() + Offsetwait);

	if(delay < 0)
		tmr.expires_at(tmr.expires_at() + Twait);
	tmr.async_wait(_strand.wrap(boost::bind(&Scheduler::_handle_timeout, this, pTmrTask, boost::asio::placeholders::error)));

#endif

}

void ELFIN::Scheduler::schedule(TimerTask* pTmrTask, long delay)
{
	schedule(pTmrTask, delay, TimerTask::COUNT_INFINITY, 0);
}




void ELFIN::Scheduler::run()
{

	boost::system::error_code ec;
	boost::asio::io_service *ios = &_strand.get_io_service();
	ios->run(ec);
	// Reset the io_service in preparation for a subsequent run() invocation.
	ios->reset();
	//_strand.get_io_service().run(ec);

	//std::cout << "error code:" << ec.value() << std::endl;
}


