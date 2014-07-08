/*
 * ELFIN.h
 *
 *  Created on: Jul 8, 2014
 *      Author: iot-team
 */

#ifndef ELFINREADER_H_
#define ELFINREADER_H_

namespace ELFIN {

// Forward declaration
class LLRPCore;
class ProgramOptions;

class ELFINReader {
public:
	ELFINReader(ProgramOptions *__progOpt);
	~ELFINReader();
	int run();
	int init();
	int connectReaderApp(StubReaderApp *_readerApp);
	void setShutdownFlag();
	int getShutdownFlag();
	int startConnection();
private:
	LLRPCore *m_LLRPCore;
};

}



#endif /* ELFINREADER_H_ */
