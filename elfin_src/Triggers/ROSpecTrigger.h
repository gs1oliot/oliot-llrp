/*
 * ROSpecTrigger.h
 *
 *  Created on: Apr 14, 2014
 *      Author: iot-team
 */

#ifndef ROSPECTRIGGER_H_
#define ROSPECTRIGGER_H_

#include "AbstractTrigger.h"
#include "../ELFIN_Platform.h"

namespace ELFIN {

class ROSpecTrigger : public AbstractTrigger {
public:
	ROSpecTrigger(LLRP::CROSpec *a_ROSpec);
	~ROSpecTrigger();
	LLRP::CROSpec *m_ROSpec;
};
}

#endif /* ROSPECTRIGGER_H_ */
