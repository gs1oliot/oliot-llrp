/**
 * @file    AbstractTrigger.h
 * @brief
 */

#ifndef __LLRP_READER__ABSTRACTTRIGGER_H__
#define __LLRP_READER__ABSTRACTTRIGGER_H__

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace ELFIN {

class AbstractTrigger {
public:
	AbstractTrigger()
	: m_isFired(0), m_timeOut(0), m_tagCount(0), m_tagCountLimit(0), m_timer(NULL) {};
	virtual ~AbstractTrigger() {
		if (m_timer != NULL) {
			delete m_timer;
			m_timer = NULL;
		};
	}
	void enableTrigger() {
		if (m_timeOut != 0) {
			createTimeout(m_timeOut);
		}
	}
	void disableTrigger() {
		m_ios.stop();
	}
	void fireTrigger() {
		m_ios.stop();
		m_isFired = 1;
	}
	int isFired() {
		return m_isFired;
	}
	void observedNewTagCallback() {
		if (m_tagCountLimit != 0) {
			m_tagCount++;
			if (m_tagCount == m_tagCountLimit) {
				fireTrigger();
			}
		}
	}
	void createTimeout(int waitMS) {
		m_timer = new boost::asio::deadline_timer(m_ios);
		m_timer->expires_from_now(boost::posix_time::milliseconds(waitMS));
		m_timer->async_wait(boost::bind(&AbstractTrigger::deadlineTimerHandler, this, boost::asio::placeholders::error));
	}
	void deadlineTimerHandler(const boost::system::error_code& e) {
		if (e != boost::asio::error::operation_aborted)
		{
			m_ios.stop();
			this->fireTrigger();
		}
	}

protected:
	int m_isFired;
	int m_timeOut;
	int m_tagCount;
	int m_tagCountLimit;
	boost::asio::io_service m_ios;
	boost::asio::deadline_timer *m_timer;
};

}

#endif /* __LLRP_READER__ABSTRACTTRIGGER_H__ */
