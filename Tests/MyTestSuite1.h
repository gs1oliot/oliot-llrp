/*
 * MyTestSuite2.h
 *
 *  Created on: Aug 29, 2013
 *      Author: shheo
 */

#ifndef MYTESTSUITE1_H_
#define MYTESTSUITE1_H_

#include <string>
#include <stdint.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <cxxtest/TestSuite.h>
#include "LLRPTestClient.h"
#include "LLRPTestClient.cpp"

class LLRPTestClient;
class TestConnection;

#include "TCR_R1.h"
#include "TCR_R3.h"
#include "TCR_R5.h"
#include "TCR_R6.h"
#include "TCR_R7.h"
#include "TCR_R8.h"
#include "TCR_R9.h"
#include "TCR_R10.h"
#include "TCR_R14.h"

class TestConnection : public CxxTest::TestSuite {
public:
	boost::thread *_pAppThread;
	LLRPTestClient *_pTestClient;
	const char * _pReaderHostName;
	void setUp() {
		printf ("\n============Starting setUp phase...============\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
		this->startApp("reader-server-mode.cfg", "config.llrp");


		_pReaderHostName = "127.0.0.1";

		_pTestClient = new LLRPTestClient(_pReaderHostName);

		//_pTestClient->m_Verbose++; // Verbose, each -v increments verbosity
		//myApp.m_Quiet++; // Quiet, don't print tag reports (used for characterization)
		//myApp.m_ModeIndex = 0; // 0~5 Select mode, if not given the default mode is used

		/*
		 * Run application, capture return value for exit status
		 */
		printf ("\n============setUp phase finished...============\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
	}

	void tearDown() {
		printf ("\n============Starting tearDown phase...============\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
		delete _pTestClient;
		system("echo 'killall stubappexample' ; killall stubappexample");
		_pAppThread->join();
		printf ("\n============tearDown phase finished...============\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
	}


	void test_TCR_R1_TCP_Connections(void)
	{
		printf ("====================TCR_R1=====================\n");
		//_pTestClient->openConnectionToReader();
		TCR_R1 m_TCR_R1(_pReaderHostName, _pTestClient);
		// Let the reader to initiate connection, for TCR_R1
		this->startApp("reader-client-mode.cfg", "config.llrp");
		printf ("================TCR_R1_Step1===================\n");
		TS_ASSERT (m_TCR_R1.TCR_R1_Step1() == 0);
		printf ("================TCR_R1_Step2===================\n");
		TS_ASSERT (m_TCR_R1.TCR_R1_Step2() == 0);
		printf ("================TCR_R1_Step3===================\n");
		TS_ASSERT (m_TCR_R1.TCR_R1_Step3() == 0);
		printf ("================TCR_R1_Step4===================\n");
		TS_ASSERT (m_TCR_R1.TCR_R1_Step4() == 0);
		printf ("================TCR_R1_Step5===================\n");
		// Start the reader as server mode, for TCR_R1
		this->startApp("reader-server-mode.cfg", "config.llrp");
		TS_ASSERT (m_TCR_R1.TCR_R1_Step5() == 0);
		printf ("================TCR_R1_Step6===================\n");
		TS_ASSERT (m_TCR_R1.TCR_R1_Step6() == 0);
		printf ("================TCR_R1_Step7===================\n");
		TS_ASSERT (m_TCR_R1.TCR_R1_Step7() == 0);
	}

	void test_TCR_R3_Get_Reader_Capabilities(void)
	{
		printf ("====================TCR_R3=====================\n");
		TCR_R3 m_TCR_R3(_pReaderHostName, _pTestClient);
		printf ("================TCR_R3_Prepare=================\n");
		TS_ASSERT (m_TCR_R3.TCR_R3_Prepare() == 0);
		printf ("================TCR_R3_Step1===================\n");
		TS_ASSERT (m_TCR_R3.TCR_R3_Step1() == 0); // FIXME: verify all capability parameters
	}

	void test_TCR_R5_Errors(void)
	{
		printf ("====================TCR_R5=====================\n");
		TCR_R5 m_TCR_R5(_pReaderHostName, _pTestClient);
		printf ("================TCR_R5_Prepare=================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Prepare() == 0);
		printf ("================TCR_R5_Step1===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step1() == 0);
		printf ("================TCR_R5_Step2===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step2() == 0);
		printf ("================TCR_R5_Step3===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step3() == 0);
		printf ("================TCR_R5_Step4===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step4() == 0);
		printf ("================TCR_R5_Step5===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step5() == 0);
		printf ("================TCR_R5_Step6===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step6() == 0);
		printf ("================TCR_R5_Step7===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step7() == 0);
		printf ("================TCR_R5_Step8===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step8() == 0);
		printf ("================TCR_R5_Step9===================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step9() == 0);
		printf ("================TCR_R5_Step10==================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step10() == 0);
		printf ("================TCR_R5_Step11==================\n");
		TS_ASSERT (m_TCR_R5.TCR_R5_Step11() == 0);
	}

	void test_TCR_R6_Reader_Operations_and_Reporting(void)
	{
		printf ("====================TCR_R6=====================\n");
		TCR_R6 m_TCR_R6(_pReaderHostName, _pTestClient);
		printf ("================TCR_R6_Prepare=================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Prepare1() == 0);
		printf ("================TCR_R6_Step1===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step1() == 0);
		printf ("================TCR_R6_Step2===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step2() == 0);
		printf ("================TCR_R6_Step3===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step3() == 0);
		printf ("================TCR_R6_Step4===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step4() == 0);
		printf ("================TCR_R6_Step5===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step5() == 0);
		printf ("================TCR_R6_Step6===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step6() == 0);
		printf ("================TCR_R6_Step7===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step7() == 0);
		printf ("================TCR_R6_Step8===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step8() == 0);
		printf ("================TCR_R6_Step9===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step9() == 0);
		printf ("================TCR_R6_Step10===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step10() == 0);
		printf ("================TCR_R6_Step11===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step11() == 0);
		printf ("================TCR_R6_Step12===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step12() == 0);
		printf ("================TCR_R6_Step13===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step13() == 0);
		printf ("================TCR_R6_Step14===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step14() == 0);
		printf ("================TCR_R6_Step15===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step15() == 0);
		printf ("================TCR_R6_Step16===================\n");
		TS_ASSERT (m_TCR_R6.TCR_R6_Step16() == 0);
	}

	void test_TCR_R7_Reader_Operations_in_Loop(void)
	{
		printf ("====================TCR_R7=====================\n");
		TCR_R7 m_TCR_R7(_pReaderHostName, _pTestClient);
		printf ("================TCR_R7_Prepare=================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Prepare() == 0);
		printf ("================TCR_R7_Step1===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step1() == 0);
		printf ("================TCR_R7_Step2===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step2() == 0);
		printf ("================TCR_R7_Step3===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step3() == 0);
		printf ("================TCR_R7_Step4===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step4() == 0);
		printf ("================TCR_R7_Step5===================\n");
		// FIXME: Need to compare with real tags. Currently just shows tags.
		TS_ASSERT (m_TCR_R7.TCR_R7_Step5() == 0);
		printf ("================TCR_R7_Step6===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step6() == 0);
		printf ("================TCR_R7_Step7===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step7() == 0);
		printf ("================TCR_R7_Step8===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step8() == 0);
		printf ("================TCR_R7_Step9===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step9() == 0);
		printf ("================TCR_R7_Step10===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step10() == 0);
		printf ("================TCR_R7_Step11===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step11() == 0);
		printf ("================TCR_R7_Step12===================\n");
		TS_ASSERT (m_TCR_R7.TCR_R7_Step12() == 0);
	}

	void test_TCR_R8_Access_Operations_and_Reporting(void)
	{
		printf ("====================TCR_R8=====================\n");
		TCR_R8 m_TCR_R8(_pReaderHostName, _pTestClient);
		printf ("================TCR_R8_Prepare=================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Prepare() == 0);
		printf ("================TCR_R8_Step1===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step1() == 0);
		printf ("================TCR_R8_Step2===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step2() == 0);
		printf ("================TCR_R8_Step3===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step3() == 0);
		printf ("================TCR_R8_Step4===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step4() == 0);
		printf ("================TCR_R8_Step5===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step5() == 0);
		printf ("================TCR_R8_Step6===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step6() == 0);
		printf ("================TCR_R8_Step6a==================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step6a() == 0);
		printf ("================TCR_R8_Step7===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step7() == 0);
		printf ("================TCR_R8_Step8===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step8() == 0);
		printf ("================TCR_R8_Step9===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step9() == 0);
		printf ("================TCR_R8_Step10===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step10() == 0);
		printf ("================TCR_R8_Step11===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step11() == 0);
		printf ("================TCR_R8_Step12===================\n");
		// This test step is invalid, because the periodic ROSpec automatically starts.
		// So just ignore and let it always return 0.
		TS_ASSERT (m_TCR_R8.TCR_R8_Step12() == 0);
		printf ("================TCR_R8_Step13===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step13() == 0);
		printf ("================TCR_R8_Step14===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step14() == 0);
		printf ("================TCR_R8_Step15===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step15() == 0);
		printf ("================TCR_R8_Step16===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step16() == 0);
		printf ("================TCR_R8_Step17===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step17() == 0);
		printf ("================TCR_R8_Step18===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step18() == 0);
		printf ("================TCR_R8_Step19===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step19() == 0);
		printf ("================TCR_R8_Step20===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step20() == 0);
		printf ("================TCR_R8_Step21===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step21() == 0);
		printf ("================TCR_R8_Step22===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step22() == 0);
		printf ("================TCR_R8_Step23===================\n");
		TS_ASSERT (m_TCR_R8.TCR_R8_Step23() == 0);
	}

	void test_TCR_R9_Tag_Observations_Count_Based_Triggering(void)
	{
		printf ("====================TCR_R9=====================\n");
		TCR_R9 m_TCR_R9(_pReaderHostName, _pTestClient);
		printf ("================TCR_R9_Prepare=================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Prepare() == 0);
		printf ("================TCR_R9_Step1===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step1() == 0);
		printf ("================TCR_R9_Step2===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step2() == 0);
		printf ("================TCR_R9_Step3===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step3() == 0);
		printf ("================TCR_R9_Step4===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step4() == 0);
		printf ("================TCR_R9_Step5===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step5() == 0);
		printf ("================TCR_R9_Step6===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step6() == 0);
		printf ("================TCR_R9_Step7===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step7() == 0);
		printf ("================TCR_R9_Step8===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step8() == 0);
		printf ("================TCR_R9_Step9===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step9() == 0);
		printf ("================TCR_R9_Step10===================\n");
		TS_ASSERT (m_TCR_R9.TCR_R9_Step10() == 0);
	}

	void test_TCR_R10_Immediate_Triggering(void)
	{
		printf ("====================TCR_R10=====================\n");
		TCR_R10 m_TCR_R10(_pReaderHostName, _pTestClient);
		printf ("================TCR_R10_Prepare=================\n");
		TS_ASSERT (m_TCR_R10.TCR_R10_Prepare() == 0);
		printf ("================TCR_R10_Step1===================\n");
		TS_ASSERT (m_TCR_R10.TCR_R10_Step1() == 0);
		printf ("================TCR_R10_Step2===================\n");
		TS_ASSERT (m_TCR_R10.TCR_R10_Step2() == 0);
		printf ("================TCR_R10_Step3===================\n");
		TS_ASSERT (m_TCR_R10.TCR_R10_Step3() == 0);
		printf ("================TCR_R10_Step4===================\n");
		TS_ASSERT (m_TCR_R10.TCR_R10_Step4() == 0);
	}

	void test_TCR_R14_Keepalives(void)
	{
		printf ("====================TCR_R14=====================\n");
		TCR_R14 m_TCR_R14(_pReaderHostName, _pTestClient);
		printf ("================TCR_R14_Prepare=================\n");
		TS_ASSERT (m_TCR_R14.TCR_R14_Prepare() == 0);
		printf ("================TCR_R14_Step1===================\n");
		TS_ASSERT (m_TCR_R14.TCR_R14_Step1() == 0);
		printf ("================TCR_R14_Step2===================\n");
		TS_ASSERT (m_TCR_R14.TCR_R14_Step2() == 0);
		printf ("================TCR_R14_Step3===================\n");
		TS_ASSERT (m_TCR_R14.TCR_R14_Step3() == 0);
		printf ("================TCR_R14_Step4===================\n");
		TS_ASSERT (m_TCR_R14.TCR_R14_Step4() == 0);
	}

	void startApp(const char *cfg, const char *llrp) {
		_pAppThread = new boost::thread(boost::bind(&TestConnection::runApp, this, cfg, llrp));
		printf ("\n============Waiting for LLRPReader to run...============\n");
		boost::this_thread::sleep_for(boost::chrono::milliseconds(3000));
	}

	void runApp(const char *cfg, const char *llrp) {
		char *cmdLine = new char[100];
		system("echo 'killall stubappexample' ; killall stubappexample");
		strcpy (cmdLine, "LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH ./stubappexample -c");
		strcat (cmdLine, cfg);
		strcat (cmdLine, " -l");
		strcat (cmdLine, llrp);
		system(cmdLine);
		delete[] cmdLine;
	}
};

#endif
/* MYTESTSUITE1_H_ */
