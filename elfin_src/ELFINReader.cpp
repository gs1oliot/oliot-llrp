/*
 * ELFIN.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: iot-team
 */

#include "LLRPCore.h"
#include "ELFINReader.h"

ELFIN::ELFINReader::ELFINReader(ProgramOptions* __progOpt)
: m_LLRPCore (new LLRPCore(__progOpt)){
}


ELFIN::ELFINReader::~ELFINReader() {
	delete this->m_LLRPCore;
}

int ELFIN::ELFINReader::run() {
	return this->m_LLRPCore->run();
}

int ELFIN::ELFINReader::init() {
	return this->m_LLRPCore->init();
}

void ELFIN::ELFINReader::setShutdownFlag() {
	this->m_LLRPCore->m_shutdown = 1;
}


int ELFIN::ELFINReader::connectReaderApp(StubReaderApp* _readerApp) {
	return this->m_LLRPCore->connectReaderApp(_readerApp);
}

int ELFIN::ELFINReader::getShutdownFlag() {
	return this->m_LLRPCore->m_shutdown;
}

int ELFIN::ELFINReader::startConnection() {
	return this->m_LLRPCore->startConnection();
}
