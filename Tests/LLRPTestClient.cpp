#include <boost/functional/hash.hpp>
#include "LLRPTestClient.h"

LLRPTestClient::LLRPTestClient(const char* pReaderHostName)
: _shutdown(0), m_Verbose(0), m_ModeIndex(-1), m_Quiet(0),
  m_nTagReport(0), m_ReaderHostName(pReaderHostName),
  m_pConnectionToReader(NULL), pTypeRegistry(NULL)
{
	/*
	 * Allocate the type registry. This is needed
	 * by the connection to decode.
	 */
	pTypeRegistry = getTheTypeRegistry();
	if(NULL == pTypeRegistry)
	{
		printf("ERROR: getTheTypeRegistry failed\n");
	}
}



int LLRPTestClient::openConnectionToReader(void) {
	CConnection *               pConn;
	int                         rc;

	if (m_pConnectionToReader != NULL) {
		delete m_pConnectionToReader;
	}

	/*
	 * Construct a connection (LLRP::CConnection).
	 * Using a 32kb max frame size for send/recv.
	 * The connection object is ready for business
	 * but not actually connected to the reader yet.
	 */
	pConn = new CConnection(pTypeRegistry, 32u*1024u, 5084);
	if(NULL == pConn)
	{
		printf("ERROR: new CConnection failed\n");
		return -2;
	}

	/*
	 * Open the connection to the reader
	 */
	if(m_Verbose)
	{
		printf ("INFO: Connecting to %s....\n", m_ReaderHostName);
	}

	rc = pConn->openConnection(m_ReaderHostName);
	if(0 != rc)
	{
		printf("ERROR: connect: %s (%d)\n", pConn->getConnectError(), rc);
		delete pConn;
		return -3;
	}

	/*
	 * Record the pointer to the connection object so other
	 * routines can use it.
	 */
	m_pConnectionToReader = pConn;

	if(m_Verbose)
	{
		printf ("INFO: Connected, checking status....\n");
	}

	return rc;
}

int LLRPTestClient::waitconenctionFromReader(void) {
	CConnection *               pConn;
	int                         rc;

	if (m_pConnectionToReader != NULL)
		delete m_pConnectionToReader;

	/*
	 * Construct a connection (LLRP::CConnection).
	 * Using a 32kb max frame size for send/recv.
	 * The connection object is ready for business
	 * but not actually connected to the reader yet.
	 */
	pConn = new CConnection(pTypeRegistry, 32u*1024u, 5084);
	if(NULL == pConn)
	{
		printf("ERROR: new CConnection failed\n");
		return -2;
	}

	/*
	 * Open the connection to the reader
	 */
	if(m_Verbose)
	{
		printf ("INFO: Connecting to %s....\n", m_ReaderHostName);
	}

	rc = pConn->acceptConnection();
	if(0 != rc)
	{
		printf("ERROR: connect: %s (%d)\n", pConn->getConnectError(), rc);
		delete pConn;
		return -3;
	}

	/*
	 * Record the pointer to the connection object so other
	 * routines can use it.
	 */
	m_pConnectionToReader = pConn;

	if(m_Verbose)
	{
		printf ("INFO: Connected, checking status....\n");
	}

	return rc;
}

int
LLRPTestClient::stop ()
{
	//int                         rc;

	if(m_Verbose)
	{
		printf("INFO: Clean up reader configuration...\n");
	}
	scrubConfiguration();

	if(m_Verbose)
	{
		printf ("INFO: Finished\n");
	}

	/*
	 * Close the connection and release its resources
	 */
	m_pConnectionToReader->closeConnection();
	delete m_pConnectionToReader;

	/*
	 * Done with the registry.
	 */
	delete pTypeRegistry;

	/*
	 * Maybe tattle on the number of tags. Normally tags are reported.
	 * The -q command option prevents that and instead asks for just a
	 * total. With the -v command option we're probably reporting tags
	 * and a final count.
	 */
	if(m_Verbose || m_Quiet)
	{
		printf("INFO: %d Tag reports\n", m_nTagReport);
	}

	/*
	 * When we get here all allocated memory should have been deallocated.
	 */

	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Await and check the connection status message from the reader
 **
 ** We are expecting a READER_EVENT_NOTIFICATION message that
 ** tells us the connection is OK. The reader is suppose to
 ** send the message promptly upon connection.
 **
 ** If there is already another LLRP connection to the
 ** reader we'll get a bad Status.
 **
 ** The message should be something like:
 **
 **     <READER_EVENT_NOTIFICATION MessageID='0'>
 **       <ReaderEventNotificationData>
 **         <UTCTimestamp>
 **           <Microseconds>1184491439614224</Microseconds>
 **         </UTCTimestamp>
 **         <ConnectionAttemptEvent>
 **           <Status>Success</Status>
 **         </ConnectionAttemptEvent>
 **       </ReaderEventNotificationData>
 **     </READER_EVENT_NOTIFICATION>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::checkConnectionStatus (void)
{
	CMessage *                  pMessage;
	CREADER_EVENT_NOTIFICATION *pNtf;
	CReaderEventNotificationData *pNtfData;
	CConnectionAttemptEvent *   pEvent;

	/*
	 * Expect the notification within 10 seconds.
	 * It is suppose to be the very first message sent.
	 */
	pMessage = recvMessage(10000);

	/*
	 * recvMessage() returns NULL if something went wrong.
	 */
	if(NULL == pMessage)
	{
		/* recvMessage already tattled */
		goto fail;
	}

	/*
	 * Check to make sure the message is of the right type.
	 * The type label (pointer) in the message should be
	 * the type descriptor for READER_EVENT_NOTIFICATION.
	 */
	if(&CREADER_EVENT_NOTIFICATION::s_typeDescriptor != pMessage->m_pType)
	{
		goto fail;
	}

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pNtf = (CREADER_EVENT_NOTIFICATION *) pMessage;
	pNtfData = pNtf->getReaderEventNotificationData();
	if(NULL == pNtfData)
	{
		goto fail;
	}

	/*
	 * The ConnectionAttemptEvent parameter must be present.
	 */
	pEvent = pNtfData->getConnectionAttemptEvent();
	if(NULL == pEvent)
	{
		goto fail;
	}

	/*
	 * The status in the ConnectionAttemptEvent parameter
	 * must indicate connection success.
	 */
	if(ConnectionAttemptStatusType_Success != pEvent->getStatus())
	{
		goto fail;
	}

	/*
	 * Done with the message
	 */
	delete pMessage;

	if(m_Verbose)
	{
		printf("INFO: Connection status OK\n");
	}

	/*
	 * Victory.
	 */
	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: checkConnectionStatus failed\n");
	delete pMessage;
	return -1;
}


/**
 *****************************************************************************
 **
 ** @brief  Scrub the reader configuration
 **
 ** The steps:
 **     - Try to reset configuration to factory defaults,
 **       this feature is optional and may not be supported
 **       by the reader.
 **     - Delete all ROSpecs
 **     - Delete all AccessSpecs
 **
 ** This deliberately has the steps as separate 'if' statements
 ** to aid easier debugging.
 **
 ** Upon successful return, the reader is as close to
 ** brand new as we can make it.
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::scrubConfiguration (void)
{
	if(0 != resetConfigurationToFactoryDefaults())
	{
		return -1;
	}

	if(0 != deleteAllROSpecs())
	{
		return -2;
	}

	if(0 != deleteAllAccessSpecs())
	{
		return -3;
	}

	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Send a SET_READER_CONFIG message that resets the
 **         reader to factory defaults.
 **
 ** NB: The ResetToFactoryDefault semantics vary between readers.
 **     It might have no effect because it is optional.
 **
 ** The message is:
 **
 **     <SET_READER_CONFIG MessageID='101'>
 **       <ResetToFactoryDefault>1</ResetToFactoryDefault>
 **     </SET_READER_CONFIG>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::resetConfigurationToFactoryDefaults (void)
{
	CSET_READER_CONFIG *        pCmd;
	CMessage *                  pRspMsg;
	CSET_READER_CONFIG_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CSET_READER_CONFIG();
	pCmd->setMessageID(101);
	pCmd->setResetToFactoryDefault(1);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a SET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CSET_READER_CONFIG_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != checkLLRPStatus(pRsp->getLLRPStatus(),
			"resetConfigurationToFactoryDefaults"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(m_Verbose)
	{
		printf("INFO: Configuration reset to factory defaults\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Delete all ROSpecs using DELETE_ROSPEC message
 **
 ** Per the spec, the DELETE_ROSPEC message contains an ROSpecID
 ** of 0 to indicate we want all ROSpecs deleted.
 **
 ** The message is
 **
 **     <DELETE_ROSPEC MessageID='102'>
 **       <ROSpecID>0</ROSpecID>
 **     </DELETE_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::deleteAllROSpecs (void)
{
	CDELETE_ROSPEC *            pCmd;
	CMessage *                  pRspMsg;
	CDELETE_ROSPEC_RESPONSE *   pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CDELETE_ROSPEC();
	pCmd->setMessageID(102);
	pCmd->setROSpecID(0);               /* All */

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a DELETE_ROSPEC_RESPONSE message.
	 */
	pRsp = (CDELETE_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != checkLLRPStatus(pRsp->getLLRPStatus(), "deleteAllROSpecs"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(m_Verbose)
	{
		printf("INFO: All ROSpecs are deleted\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Delete all AccessSpecs using DELETE_ACCESSSPEC message
 **
 ** Per the spec, the DELETE_ACCESSSPEC message contains an AccessSpecID
 ** of 0 to indicate we want all AccessSpecs deleted.
 **
 ** The message is
 **
 **     <DELETE_ACCESSSPEC MessageID='103'>
 **       <AccessSpecID>0</AccessSpecID>
 **     </DELETE_ACCESSSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::deleteAllAccessSpecs (void)
{
	CDELETE_ACCESSSPEC *        pCmd;
	CMessage *                  pRspMsg;
	CDELETE_ACCESSSPEC_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CDELETE_ACCESSSPEC();
	pCmd->setMessageID(103);
	pCmd->setAccessSpecID(0);           /* All */

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a DELETE_ACCESSSPEC_RESPONSE message.
	 */
	pRsp = (CDELETE_ACCESSSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != checkLLRPStatus(pRsp->getLLRPStatus(), "deleteAllAccessSpecs"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(m_Verbose)
	{
		printf("INFO: All AccessSpecs are deleted\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}

/**
 *****************************************************************************
 **
 ** @brief  Get the reader capabilities using GET_READER_CAPABILTIES message
 **
 ** The RequestedData selects all capabilities.
 ** It's mostly here as an example.
 ** If you run this program with -vv (verbose level 2)
 ** you can look at the XML of the capabilities that
 ** come back from the reader.
 **
 ** The message is
 **
 **     <GET_READER_CAPABILITIES MessageID='120'>
 **       <RequestedData>All</RequestedData>
 **     </GET_READER_CAPABILITIES>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::getAllCapabilities (void)
{
	CGET_READER_CAPABILITIES *  pCmd;
	CMessage *                  pRspMsg;
	CGET_READER_CAPABILITIES_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CGET_READER_CAPABILITIES();
	pCmd->setMessageID(120);
	pCmd->setRequestedData(GetReaderCapabilitiesRequestedData_All);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a GET_READER_CAPABILITIES_RESPONSE message.
	 */
	pRsp = (CGET_READER_CAPABILITIES_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != checkLLRPStatus(pRsp->getLLRPStatus(), "getAllCapabilities"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(m_Verbose)
	{
		printf("INFO: Got capabilities\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Get the reader configuration using GET_READER_CONFIG message
 **
 ** The RequestedData selects all configuration parameters.
 ** It's mostly here as an example.
 ** If you run this program with -vv (verbose level 2)
 ** you can look at the XML of the configuration that
 ** comes back from the reader.
 **
 ** The message is
 **
 **     <GET_READER_CONFIG MessageID='121'>
 **       <RequestedData>All</RequestedData>
 **     </GET_READER_CONFIG>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::getAllConfiguration (void)
{
	CGET_READER_CONFIG *        pCmd;
	CMessage *                  pRspMsg;
	CGET_READER_CONFIG_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CGET_READER_CONFIG();
	pCmd->setMessageID(121);
	pCmd->setRequestedData(GetReaderConfigRequestedData_All);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CGET_READER_CONFIG_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != checkLLRPStatus(pRsp->getLLRPStatus(), "getAllConfiguration"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(m_Verbose)
	{
		printf("INFO: Got configuration\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Configure event notifications using SET_READER_CONFIG message
 **
 ** The events we want are:
 **     - Hopping events when the reader changes channel
 **     - ROSpec events, when ROSpecs start and stop
 **     - AISpec events, when AISpecs stop
 **     - Antenna events, when an antenna fails
 **     - Buffer events, so we can see when we are not keeping up
 **     - Reader exceptions, so we can see when something goes wrong
 **
 ** This is a good example of how to do a list of subparameters.
 **
 ** The message is
 **
 **     <SET_READER_CONFIG MessageID='150'>
 **       <ResetToFactoryDefault>0</ResetToFactoryDefault>
 **       <ReaderEventNotificationSpec>
 **         <EventNotificationState>
 **           <EventType>Upon_Hopping_To_Next_Channel</EventType>
 **           <NotificationState>1</NotificationState>
 **         </EventNotificationState>
 **         <EventNotificationState>
 **           <EventType>GPI_Event</EventType>
 **           <NotificationState>0</NotificationState>
 **         </EventNotificationState>
 **         <EventNotificationState>
 **           <EventType>ROSpec_Event</EventType>
 **           <NotificationState>1</NotificationState>
 **         </EventNotificationState>
 **         <EventNotificationState>
 **           <EventType>Report_Buffer_Fill_Warning</EventType>
 **           <NotificationState>1</NotificationState>
 **         </EventNotificationState>
 **         <EventNotificationState>
 **           <EventType>Reader_Exception_Event</EventType>
 **           <NotificationState>1</NotificationState>
 **         </EventNotificationState>
 **         <EventNotificationState>
 **           <EventType>RFSurvey_Event</EventType>
 **           <NotificationState>0</NotificationState>
 **         </EventNotificationState>
 **         <EventNotificationState>
 **           <EventType>AISpec_Event</EventType>
 **           <NotificationState>1</NotificationState>
 **         </EventNotificationState>
 **         <EventNotificationState>
 **           <EventType>AISpec_Event_With_Details</EventType>
 **           <NotificationState>0</NotificationState>
 **         </EventNotificationState>
 **         <EventNotificationState>
 **           <EventType>Antenna_Event</EventType>
 **           <NotificationState>1</NotificationState>
 **         </EventNotificationState>
 **       </ReaderEventNotificationSpec>
 **     </SET_READER_CONFIG>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::configureNotificationStates (void)
{
	CEventNotificationState *   pENHop = new CEventNotificationState();
	pENHop->setEventType(NotificationEventType_Upon_Hopping_To_Next_Channel);
	pENHop->setNotificationState(TRUE);

	CEventNotificationState *   pENGPI = new CEventNotificationState();
	pENGPI->setEventType(NotificationEventType_GPI_Event);
	pENGPI->setNotificationState(FALSE);

	CEventNotificationState *   pENROSpec = new CEventNotificationState();
	pENROSpec->setEventType(NotificationEventType_ROSpec_Event);
	pENROSpec->setNotificationState(TRUE);

	CEventNotificationState *   pENBufWarn = new CEventNotificationState();
	pENBufWarn->setEventType(NotificationEventType_Report_Buffer_Fill_Warning);
	pENBufWarn->setNotificationState(TRUE);

	CEventNotificationState *   pENExcept = new CEventNotificationState();
	pENExcept->setEventType(NotificationEventType_Reader_Exception_Event);
	pENExcept->setNotificationState(TRUE);

	CEventNotificationState *   pENSurvey = new CEventNotificationState();
	pENSurvey->setEventType(NotificationEventType_RFSurvey_Event);
	pENSurvey->setNotificationState(FALSE);

	CEventNotificationState *   pENAISpec = new CEventNotificationState();
	pENAISpec->setEventType(NotificationEventType_AISpec_Event);
	pENAISpec->setNotificationState(TRUE);

	CEventNotificationState *   pENAISpecDetails
	= new CEventNotificationState();
	pENAISpecDetails->setEventType(
			NotificationEventType_AISpec_Event_With_Details);
	pENAISpecDetails->setNotificationState(FALSE);

	CEventNotificationState *   pENAnt = new CEventNotificationState();
	pENAnt->setEventType(NotificationEventType_Antenna_Event);
	pENAnt->setNotificationState(TRUE);

	CReaderEventNotificationSpec * pReaderEventNotificationSpec;

	pReaderEventNotificationSpec = new CReaderEventNotificationSpec;

	pReaderEventNotificationSpec->addEventNotificationState(pENHop);
	pReaderEventNotificationSpec->addEventNotificationState(pENGPI);
	pReaderEventNotificationSpec->addEventNotificationState(pENROSpec);
	pReaderEventNotificationSpec->addEventNotificationState(pENBufWarn);
	pReaderEventNotificationSpec->addEventNotificationState(pENExcept);
	pReaderEventNotificationSpec->addEventNotificationState(pENSurvey);
	pReaderEventNotificationSpec->addEventNotificationState(pENAISpec);
	pReaderEventNotificationSpec->addEventNotificationState(
			pENAISpecDetails);
	pReaderEventNotificationSpec->addEventNotificationState(pENAnt);

	CSET_READER_CONFIG *        pCmd;
	CMessage *                  pRspMsg;
	CSET_READER_CONFIG_RESPONSE *pRsp;

	/*
	 * Compose the command message.
	 * N.B.: After the message is composed, all the parameters
	 *       constructed, immediately above, are considered "owned"
	 *       by the command message. When it is destructed so
	 *       too will the parameters be.
	 */
	pCmd = new CSET_READER_CONFIG();
	pCmd->setMessageID(150);
	pCmd->setReaderEventNotificationSpec(pReaderEventNotificationSpec);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = transact(pCmd);

	/*
	 * Done with the command message.
	 * N.B.: And the parameters
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a SET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CSET_READER_CONFIG_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != checkLLRPStatus(pRsp->getLLRPStatus(),
			"configureNotificationStates"))
	{
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(m_Verbose)
	{
		printf("INFO: Notifications are configured\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Add our ROSpec using ADD_ROSPEC message
 **
 ** This ROSpec waits for a START_ROSPEC message,
 ** then takes inventory on all antennas until no new tags have
 ** been seen in 3.5 seconds. The operation never runs longer
 ** than 12.5 seconds.
 **
 ** The tag report is generated after the ROSpec is done.
 **
 ** This example is deliberately streamlined.
 ** Nothing here configures the antennas, RF, or Gen2.
 ** The current defaults are used. Remember we just reset
 ** the reader to factory defaults (above). Normally an
 ** application would be more precise in configuring the
 ** reader and in its ROSpecs.
 **
 ** Experience suggests that typical ROSpecs are about
 ** double this in size.
 **
 ** The message is
 **
 **     <ADD_ROSPEC MessageID='201'>
 **       <ROSpec>
 **         <ROSpecID>123</ROSpecID>
 **         <Priority>0</Priority>
 **         <CurrentState>Disabled</CurrentState>
 **         <ROBoundarySpec>
 **           <ROSpecStartTrigger>
 **             <ROSpecStartTriggerType>Null</ROSpecStartTriggerType>
 **           </ROSpecStartTrigger>
 **           <ROSpecStopTrigger>
 **             <ROSpecStopTriggerType>Null</ROSpecStopTriggerType>
 **             <DurationTriggerValue>0</DurationTriggerValue>
 **           </ROSpecStopTrigger>
 **         </ROBoundarySpec>
 **         <AISpec>
 **           <AntennaIDs>0</AntennaIDs>
 **           <AISpecStopTrigger>
 **             <AISpecStopTriggerType>Duration</AISpecStopTriggerType>
 **             <DurationTrigger>30000</DurationTrigger>
 **           </AISpecStopTrigger>
 **           <InventoryParameterSpec>
 **             <InventoryParameterSpecID>1234</InventoryParameterSpecID>
 **             <ProtocolID>EPCGlobalClass1Gen2</ProtocolID>
 **             <AntennaConfiguration>
 **               <AntennaID>0</AntennaID>
 **               <C1G2InventoryCommand>
 **                 <TagInventoryStateAware>false</TagInventoryStateAware>
 ** BEGIN C1G2RFControl only if mode specified on command line
 **                  <C1G2RFControl>
 **                   <ModeIndex>$MODE</ModeIndex>
 **                   <Tari>0</Tari>
 **                 </C1G2RFControl>
 ** END   C1G2RFControl only if mode specified on command line
 **                 <C1G2SingulationControl>
 **                   <Session>2</Session>
 **                   <TagPopulation>32</TagPopulation>
 **                   <TagTransitTime>0</TagTransitTime>
 **                 </C1G2SingulationControl>
 **               </C1G2InventoryCommand>
 **             </AntennaConfiguration>
 **           </InventoryParameterSpec>
 **         </AISpec>
 **         <ROReportSpec>
 **           <ROReportTrigger>Upon_N_Tags_Or_End_Of_ROSpec</ROReportTrigger>
 **           <N>0</N>
 **           <TagReportContentSelector>
 **             <EnableROSpecID>0</EnableROSpecID>
 **             <EnableSpecIndex>0</EnableSpecIndex>
 **             <EnableInventoryParameterSpecID>0</EnableInventoryParameterSpecID>
 **             <EnableAntennaID>1</EnableAntennaID>
 **             <EnableChannelIndex>0</EnableChannelIndex>
 **             <EnablePeakRSSI>1</EnablePeakRSSI>
 **             <EnableFirstSeenTimestamp>1</EnableFirstSeenTimestamp>
 **             <EnableLastSeenTimestamp>1</EnableLastSeenTimestamp>
 **             <EnableTagSeenCount>1</EnableTagSeenCount>
 **             <EnableAccessSpecID>0</EnableAccessSpecID>
 **           </TagReportContentSelector>
 **         </ROReportSpec>
 **       </ROSpec>
 **     </ADD_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::addROSpec (void)
{
	CROSpecStartTrigger *       pROSpecStartTrigger =
			new CROSpecStartTrigger();
	pROSpecStartTrigger->setROSpecStartTriggerType(
			ROSpecStartTriggerType_Null);

	CROSpecStopTrigger *        pROSpecStopTrigger = new CROSpecStopTrigger();
	pROSpecStopTrigger->setROSpecStopTriggerType(ROSpecStopTriggerType_Null);
	pROSpecStopTrigger->setDurationTriggerValue(0);     /* n/a */

	CROBoundarySpec *           pROBoundarySpec = new CROBoundarySpec();
	pROBoundarySpec->setROSpecStartTrigger(pROSpecStartTrigger);
	pROBoundarySpec->setROSpecStopTrigger(pROSpecStopTrigger);

#if 1
	CAISpecStopTrigger *        pAISpecStopTrigger = new CAISpecStopTrigger();
	pAISpecStopTrigger->setAISpecStopTriggerType(
			AISpecStopTriggerType_Duration);
	pAISpecStopTrigger->setDurationTrigger(30000);
#else
	CTagObservationTrigger *    pTagObservationTrigger =
			new CTagObservationTrigger();
	pTagObservationTrigger->setTriggerType (
			TagObservationTriggerType_Upon_Seeing_No_More_New_Tags_For_Tms_Or_Timeout);
	pTagObservationTrigger->setNumberOfTags(0);         /* unlimited */
	pTagObservationTrigger->setNumberOfAttempts(0);     /* n/a */
	pTagObservationTrigger->setT(3500);                 /* 3.5 sec */
	pTagObservationTrigger->setTimeout(12500);          /* 12.5 sec */

	CAISpecStopTrigger *        pAISpecStopTrigger = new CAISpecStopTrigger();
	pAISpecStopTrigger->setAISpecStopTriggerType(
			AISpecStopTriggerType_Tag_Observation);
	pAISpecStopTrigger->setDurationTrigger(0);
	pAISpecStopTrigger->setTagObservationTrigger(pTagObservationTrigger);
#endif

	CC1G2SingulationControl *   pC1G2SingulationControl =
			new CC1G2SingulationControl();
	pC1G2SingulationControl->setSession(2);
	pC1G2SingulationControl->setTagPopulation(32);

	CC1G2InventoryCommand *     pC1G2InventoryCommand =
			new CC1G2InventoryCommand();
	pC1G2InventoryCommand->setC1G2SingulationControl(
			pC1G2SingulationControl);
	/*
	 * C1G2RFControl might be set below if a m_ModeIndex was
	 * given on the command line.
	 */
	CAntennaConfiguration *     pAntennaConfiguration =
			new CAntennaConfiguration();
	pAntennaConfiguration->addAirProtocolInventoryCommandSettings (
			pC1G2InventoryCommand);

	CInventoryParameterSpec *   pInventoryParameterSpec =
			new CInventoryParameterSpec();
	pInventoryParameterSpec->setInventoryParameterSpecID(1234);
	pInventoryParameterSpec->setProtocolID(AirProtocols_EPCGlobalClass1Gen2);
	pInventoryParameterSpec->addAntennaConfiguration (
			pAntennaConfiguration);

	llrp_u16v_t                 AntennaIDs = llrp_u16v_t(1);
	AntennaIDs.m_pValue[0] = 0;         /* All */

	CAISpec *                   pAISpec = new CAISpec();
	pAISpec->setAntennaIDs(AntennaIDs);
	pAISpec->setAISpecStopTrigger(pAISpecStopTrigger);
	pAISpec->addInventoryParameterSpec(pInventoryParameterSpec);

	CTagReportContentSelector * pTagReportContentSelector =
			new CTagReportContentSelector();
	pTagReportContentSelector->setEnableROSpecID(FALSE);
	pTagReportContentSelector->setEnableSpecIndex(FALSE);
	pTagReportContentSelector->setEnableInventoryParameterSpecID(FALSE);
	pTagReportContentSelector->setEnableAntennaID(TRUE);
	pTagReportContentSelector->setEnableChannelIndex(FALSE);
	pTagReportContentSelector->setEnablePeakRSSI(TRUE);
	pTagReportContentSelector->setEnableFirstSeenTimestamp(TRUE);
	pTagReportContentSelector->setEnableLastSeenTimestamp(TRUE);
	pTagReportContentSelector->setEnableTagSeenCount(TRUE);
	pTagReportContentSelector->setEnableAccessSpecID(FALSE);

	CROReportSpec *             pROReportSpec = new CROReportSpec();
	pROReportSpec->setROReportTrigger(
			ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec);
	pROReportSpec->setN(1);         /* Report every singulation */
	pROReportSpec->setTagReportContentSelector(pTagReportContentSelector);

	CROSpec *                   pROSpec = new CROSpec();
	pROSpec->setROSpecID(123);
	pROSpec->setPriority(0);
	pROSpec->setCurrentState(ROSpecState_Disabled);
	pROSpec->setROBoundarySpec(pROBoundarySpec);
	pROSpec->addSpecParameter(pAISpec);
	pROSpec->setROReportSpec(pROReportSpec);

	CADD_ROSPEC *               pCmd;
	CMessage *                  pRspMsg;
	CADD_ROSPEC_RESPONSE *      pRsp;

	/*
	 * Compose the command message.
	 * N.B.: After the message is composed, all the parameters
	 *       constructed, immediately above, are considered "owned"
	 *       by the command message. When it is destructed so
	 *       too will the parameters be.
	 */
	pCmd = new CADD_ROSPEC();
	pCmd->setMessageID(201);
	pCmd->setROSpec(pROSpec);

	/*
	 * If the mode was specified link in the AntennaConfiguration
	 * that was prepared, above. The absence of the AntennaConfiguration
	 * tells the reader to use the default mode.
	 */
	if(0 <= m_ModeIndex)
	{
		CC1G2RFControl *        pC1G2RFControl =
				new CC1G2RFControl();

		pC1G2RFControl->setModeIndex(m_ModeIndex);
		pC1G2InventoryCommand->setC1G2RFControl(pC1G2RFControl);
	}

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = transact(pCmd);

	/*
	 * Done with the command message.
	 * N.B.: And the parameters
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a ADD_ROSPEC_RESPONSE message.
	 */
	pRsp = (CADD_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != checkLLRPStatus(pRsp->getLLRPStatus(), "addROSpec"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(m_Verbose)
	{
		printf("INFO: ROSpec added\n");
	}

	/*
	 * Victory.
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Enable our ROSpec using ENABLE_ROSPEC message
 **
 ** Enable the ROSpec that was added above.
 **
 ** The message we send is:
 **     <ENABLE_ROSPEC MessageID='202'>
 **       <ROSpecID>123</ROSpecID>
 **     </ENABLE_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::enableROSpec (llrp_u32_t ROSpecID, const char *testName)
{
	CENABLE_ROSPEC *            pCmd;
	CMessage *                  pRspMsg;
	CENABLE_ROSPEC_RESPONSE *   pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CENABLE_ROSPEC();
	pCmd->setMessageID(202);
	pCmd->setROSpecID(ROSpecID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a ENABLE_ROSPEC_RESPONSE message.
	 */
	pRsp = (CENABLE_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != checkLLRPStatus(pRsp->getLLRPStatus(), testName))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Tattle progress, maybe
	 */
	if(m_Verbose)
	{
		printf("INFO: %s passed\n", testName);
	}

	/*
	 * Victory.
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Start the ROSpec using START_ROSPEC message then
 **         monitor the operation until it is done
 **
 ** The steps:
 **     - Send the START_ROSPEC message, but not using transact()
 **     - Receive all messages coming back from the reader
 **     - We can be absolutely sure of the order the messages will arrive
 **         - Expect a START_ROSPEC_RESPONSE message
 **         - Expect a READER_EVENT_NOTIFICATION with
 **           a ROSpecEvent parameter indicating that the
 **           ROSpec started
 **         - Expect many READER_EVENT_NOTIFICATION messages each
 **           with HoppingEvent parameters as the reader
 **           tries different channels. These are not specifically
 **           handled here. If you run this program with -vv
 **           (verbose level 2) you can look at the XML of the
 **           configuration that comes back from the reader.
 **         - Expect at least one RO_ACCESS_REPORT message
 **           and print the tag report
 **         - Expect a READER_EVENT_NOTIFICATION with
 **           a ROSpecEvent parameter indicating that the
 **           ROSpec stopped. This is when the loop exits.
 **
 **
 ** This could use transact() to send the START_ROSPEC.
 ** The LLRP::CConnection mechanism preserves notifications
 ** while looking for response messages, like
 ** START_ROSPEC_RESPONSE. Afterward the notifications
 ** can be received.
 **
 ** This example shows how to just send a message and
 ** then combine fielding the resposne with fielding
 ** notifications.
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
LLRPTestClient::startAndMonitorOperation (void)
{
	//int                         bDone = 0;
	int                         RetVal = 0;

	/*
	 * Keep receiving messages until done or until
	 * something bad happens.
	 */
	while(!_shutdown)
	{
		CMessage *              pMessage;
		const CTypeDescriptor * pType;

		/*
		 * Wait up to 20 seconds for a message. They should
		 * be much more frequent because there will be several
		 * hopping events per second.
		 */
		pMessage = recvMessage(20000);
		if(NULL == pMessage)
		{
			/*
			 * Did not receive a message within a reasonable
			 * amount of time. recvMessage() already tattled
			 */
			RetVal = -2;
			_shutdown = 1;
			continue;
		}

		/*
		 * What happens depends on what kind of message
		 * received. Use the type label (m_pType) to
		 * discriminate message types.
		 */
		pType = pMessage->m_pType;

		/*
		 * Is it the response from the START_ROSPEC?
		 * If so, just make sure the status is good.
		 * If the status is not good there is no
		 * ROSpec running and things are done.
		 */
		if(&CSTART_ROSPEC_RESPONSE::s_typeDescriptor == pType)
		{
			CSTART_ROSPEC_RESPONSE *pRsp;

			/*
			 * Cast to a START_ROSPEC_RESPONSE message.
			 */
			pRsp = (CSTART_ROSPEC_RESPONSE *) pMessage;

			/*
			 * Check the LLRPStatus parameter.
			 */
			if(0 != checkLLRPStatus(pRsp->getLLRPStatus(), "startROSpec"))
			{
				/* checkLLRPStatus already tattled */
				RetVal = -3;
				_shutdown = 1;
			}
			else
			{
				/* Good start. Tattle, maybe. */
				if(m_Verbose)
				{
					printf("INFO: Start command acknowledged\n");
				}
			}
		}

		/*
		 * Is it a reader event? If so, which ones?
		 */
		else if(&CREADER_EVENT_NOTIFICATION::s_typeDescriptor == pType)
		{
			CREADER_EVENT_NOTIFICATION *pNtf;
			CReaderEventNotificationData *pNtfData;

			pNtf = (CREADER_EVENT_NOTIFICATION *) pMessage;
			pNtfData = pNtf->getReaderEventNotificationData();

			/*
			 * Is there an ROSpecEvent? These tell use when the ROSpec
			 * starts and stops. When it stops things are done.
			 */
			if(NULL != pNtfData->getROSpecEvent())
			{
				CROSpecEvent *    pROEvent;

				pROEvent = pNtfData->getROSpecEvent();
				switch(pROEvent->getEventType())
				{
				default:
					printf("ERROR: Unknown RO event type\n");
					RetVal = -4;
					_shutdown = 1;
					break;

				case ROSpecEventType_Start_Of_ROSpec:
					if(m_Verbose)
					{
						printf("INFO: RO running\n");
					}
					break;

				case ROSpecEventType_End_Of_ROSpec:
					if(m_Verbose)
					{
						printf("INFO: RO stopped\n");
					}
					RetVal = 0;
					_shutdown = 1;
					break;
				}
			}

			/*
			 * Other reader events may be present.
			 * Notifications enabled include AISpec events,
			 * antenna events, and hopping events.
			 * Run this programs with -vv to see the
			 * XML for the other events.
			 *
			 * For this example we just ignore them.
			 */
		}

		/*
		 * Is it a tag report? If so, print it out.
		 */
		else if(&CRO_ACCESS_REPORT::s_typeDescriptor == pType)
		{
			CRO_ACCESS_REPORT * pNtf;

			pNtf = (CRO_ACCESS_REPORT *) pMessage;

			printTagReportData(pNtf);
		}

		/*
		 * Hmmm. Something unexpected. Just tattle and keep going.
		 */
		else
		{
			printf("WARNING: Ignored unexpected message during monitor: %s\n",
					pType->m_pName);
		}

		/*
		 * Done with the received message
		 */
		delete pMessage;
	}

	/*
	 * Tattle about how things turned out.
	 */
	if(RetVal != 0)
	{
		printf("ERROR: Inventory operation terminated (%d)\n", RetVal);
	}
	else if(m_Verbose)
	{
		printf("INFO: Inventory operation finished\n");
	}

	return RetVal;
}


/**
 *****************************************************************************
 **
 ** @brief  Helper routine to print a tag report
 **
 ** The report is printed in list order, which is arbitrary.
 **
 ** TODO: It would be cool to sort the list by EPC and antenna,
 **       then print it.
 **
 ** @return     void
 **
 *****************************************************************************/

void
LLRPTestClient::printTagReportData (
		CRO_ACCESS_REPORT *           pRO_ACCESS_REPORT)
{
	std::list<CTagReportData *>::iterator Cur;
	unsigned int                nEntry = 0;

	/*
	 * Count the number of tag reports.
	 */
	m_nTagReport++;

	/*
	 * If individual tag reports are not wanted, just return.
	 */
	if(m_Quiet)
	{
		return;
	}

	/*
	 * Loop through and count the number of entries
	 */
	for (
			Cur = pRO_ACCESS_REPORT->beginTagReportData();
			Cur != pRO_ACCESS_REPORT->endTagReportData();
			Cur++)
	{
		nEntry++;
	}

	printf("INFO: %u tag report entries\n", nEntry);

	/*
	 * Loop through again and print each entry.
	 */
	for (
			Cur = pRO_ACCESS_REPORT->beginTagReportData();
			Cur != pRO_ACCESS_REPORT->endTagReportData();
			Cur++)
	{
		printOneTagReportData (*Cur);
	}
}


/**
 *****************************************************************************
 **
 ** @brief  Helper routine to print one tag report entry on one line
 **
 ** The format is, approximately:
 **
 **     1234-5678-9ABC-DEF0-1234-5678  ant1  -55dBm   27seen
 **
 ** @return     void
 **
 *****************************************************************************/

void
LLRPTestClient::printOneTagReportData (
		CTagReportData *              pTagReportData)
{
	const CTypeDescriptor *     pType;
	char                        aBuf[64];

	/*
	 * Print the EPC. It could be an 96-bit EPC_96 parameter
	 * or an variable length EPCData parameter.
	 */

	CParameter *                pEPCParameter =
			pTagReportData->getEPCParameter();

	if(NULL != pEPCParameter)
	{
		char *              p = aBuf;
		llrp_u96_t          my_u96;
		llrp_u1v_t          my_u1v;
		llrp_u8_t *         pValue = NULL;
		unsigned int        n, i;

		pType = pEPCParameter->m_pType;
		if(&CEPC_96::s_typeDescriptor == pType)
		{
			CEPC_96             *pEPC_96;

			pEPC_96 = (CEPC_96 *) pEPCParameter;
			my_u96 = pEPC_96->getEPC();
			pValue = my_u96.m_aValue;
			n = 12u;
		}
		else if(&CEPCData::s_typeDescriptor == pType)
		{
			CEPCData *          pEPCData;

			pEPCData = (CEPCData *) pEPCParameter;
			my_u1v = pEPCData->getEPC();
			pValue = my_u1v.m_pValue;
			n = (my_u1v.m_nBit + 7u) / 8u;
		}

		if(NULL != pValue)
		{
			for(i = 0; i < n; i++)
			{
				if(0 < i && i%2 == 0)
				{
					*p++ = '-';
				}
				sprintf(p, "%02X", pValue[i]);
				while(*p) p++;
			}
		}
		else
		{
			strcpy(aBuf, "---unknown-epc-data-type---");
		}
	}
	else
	{
		strcpy(aBuf, "---missing-epc-data---");
	}
	printf("%-32s", aBuf);

	/*
	 * Print the antenna ID number
	 */
	if(NULL != pTagReportData->getAntennaID())
	{
		sprintf(aBuf, "ant%d",
				pTagReportData->getAntennaID()->getAntennaID());
	}
	else
	{
		sprintf(aBuf, "ant?");
	}
	printf(" %-5s", aBuf);

	/*
	 * Print the peak RSSI
	 */
	if(NULL != pTagReportData->getPeakRSSI())
	{
		sprintf(aBuf, "%3ddBm",
				pTagReportData->getPeakRSSI()->getPeakRSSI());
	}
	else
	{
		sprintf(aBuf, "  ?dBm");
	}
	printf(" %-7s", aBuf);

	/*
	 * Print the number of times the tag was seen
	 */
	if(NULL != pTagReportData->getTagSeenCount())
	{
		sprintf(aBuf, "%4dseen",
				pTagReportData->getTagSeenCount()->getTagCount());
	}
	else
	{
		sprintf(aBuf, "   ?seen");
	}
	printf(" %-8s", aBuf);

	/*
	 * End of line
	 */
	printf("\n");
}


/**
 *****************************************************************************
 **
 ** @brief  Helper routine to check an LLRPStatus parameter
 **         and tattle on errors
 **
 ** Helper routine to interpret the LLRPStatus subparameter
 ** that is in all responses. It tattles on an error, if one,
 ** and tries to safely provide details.
 **
 ** This simplifies the code, above, for common check/tattle
 ** sequences.
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong, already tattled
 **
 *****************************************************************************/

int
LLRPTestClient::checkLLRPStatus (
		CLLRPStatus *                 pLLRPStatus,
		const char *                        pWhatStr)
{
	/*
	 * The LLRPStatus parameter is mandatory in all responses.
	 * If it is missing there should have been a decode error.
	 * This just makes sure (remember, this program is a
	 * diagnostic and suppose to catch LTKC mistakes).
	 */
	if(NULL == pLLRPStatus)
	{
		printf("ERROR: %s missing LLRP status\n", pWhatStr);
		return -1;
	}

	/*
	 * Make sure the status is M_Success.
	 * If it isn't, print the error string if one.
	 * This does not try to pretty-print the status
	 * code. To get that, run this program with -vv
	 * and examine the XML output.
	 */
	if(StatusCode_M_Success != pLLRPStatus->getStatusCode())
	{
		llrp_utf8v_t            ErrorDesc;

		ErrorDesc = pLLRPStatus->getErrorDescription();

		if(0 == ErrorDesc.m_nValue)
		{
			printf("ERROR: %s failed, no error description given\n",
					pWhatStr);
		}
		else
		{
			printf("ERROR: %s failed, %.*s\n",
					pWhatStr, ErrorDesc.m_nValue, ErrorDesc.m_pValue);
		}
		return -2;
	}

	/*
	 * Victory. Everything is fine.
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to do an LLRP transaction
 **
 ** Wrapper to transact a request/resposne.
 **     - Print the outbound message in XML if verbose level is at least 2
 **     - Send it using the LLRP_Conn_transact()
 **     - LLRP_Conn_transact() receives the response or recognizes an error
 **     - Tattle on errors, if any
 **     - Print the received message in XML if verbose level is at least 2
 **     - If the response is ERROR_MESSAGE, the request was sufficiently
 **       misunderstood that the reader could not send a proper reply.
 **       Deem this an error, free the message.
 **
 ** The message returned resides in allocated memory. It is the
 ** caller's obligtation to free it.
 **
 ** @return     ==NULL          Something went wrong, already tattled
 **             !=NULL          Pointer to a message
 **
 *****************************************************************************/

CMessage *
LLRPTestClient::transact (
		CMessage *                    pSendMsg)
{
	CConnection *               pConn = m_pConnectionToReader;
	CMessage *                  pRspMsg;

	/*
	 * Print the XML text for the outbound message if
	 * verbosity is 2 or higher.
	 */
	if(1 < m_Verbose)
	{
		/* If -qq command option, do XML encode but don't actually print */
		if(2 > m_Quiet)
		{
			printf("\n===================================\n");
			printf("INFO: Transact sending\n");
		}
		printXMLMessage(pSendMsg);
	}

	/*
	 * Send the message, expect the response of certain type.
	 * If LLRP::CConnection::transact() returns NULL then there was
	 * an error. In that case we try to print the error details.
	 */
	pRspMsg = pConn->transact(pSendMsg, 5000);

	if(NULL == pRspMsg)
	{
		const CErrorDetails *   pError = pConn->getTransactError();

		printf("ERROR: %s transact failed, %s\n",
				pSendMsg->m_pType->m_pName,
				pError->m_pWhatStr ? pError->m_pWhatStr : "no reason given");

		if(NULL != pError->m_pRefType)
		{
			printf("ERROR: ... reference type %s\n",
					pError->m_pRefType->m_pName);
		}

		if(NULL != pError->m_pRefField)
		{
			printf("ERROR: ... reference field %s\n",
					pError->m_pRefField->m_pName);
		}

		return NULL;
	}

	/*
	 * Print the XML text for the inbound message if
	 * verbosity is 2 or higher.
	 */
	if(1 < m_Verbose)
	{
		/* If -qq command option, do XML encode but don't actually print */
		if(2 > m_Quiet)
		{
			printf("\n- - - - - - - - - - - - - - - - - -\n");
			printf("INFO: Transact received response\n");
		}
		printXMLMessage(pRspMsg);
	}

	/*
	 * If it is an ERROR_MESSAGE (response from reader
	 * when it can't understand the request), tattle
	 * and declare defeat.
	 */
	if(&CERROR_MESSAGE::s_typeDescriptor == pRspMsg->m_pType)
	{
		const CTypeDescriptor * pResponseType;

		pResponseType = pSendMsg->m_pType->m_pResponseType;

		printf("ERROR: Received ERROR_MESSAGE instead of %s\n",
				pResponseType->m_pName);
		delete pRspMsg;
		pRspMsg = NULL;
	}

	return pRspMsg;
}


/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to receive a message
 **
 ** This can receive notifications as well as responses.
 **     - Recv a message using the LLRP_Conn_recvMessage()
 **     - Tattle on errors, if any
 **     - Print the message in XML if verbose level is at least 2
 **
 ** The message returned resides in allocated memory. It is the
 ** caller's obligtation to free it.
 **
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 **
 ** @return     ==NULL          Something went wrong, already tattled
 **             !=NULL          Pointer to a message
 **
 *****************************************************************************/

CMessage *
LLRPTestClient::recvMessage (
		int                           nMaxMS)
{
	CConnection *               pConn = m_pConnectionToReader;
	CMessage *                  pMessage;

	/*
	 * Receive the message subject to a time limit
	 */
	pMessage = pConn->recvMessage(nMaxMS);

	/*
	 * If LLRP::CConnection::recvMessage() returns NULL then there was
	 * an error. In that case we try to print the error details.
	 */
	if(NULL == pMessage)
	{
		const CErrorDetails *   pError = pConn->getRecvError();

		printf("ERROR: recvMessage failed, %s\n",
				pError->m_pWhatStr ? pError->m_pWhatStr : "no reason given");

		if(NULL != pError->m_pRefType)
		{
			printf("ERROR: ... reference type %s\n",
					pError->m_pRefType->m_pName);
		}

		if(NULL != pError->m_pRefField)
		{
			printf("ERROR: ... reference field %s\n",
					pError->m_pRefField->m_pName);
		}

		return NULL;
	}

	/*
	 * Print the XML text for the inbound message if
	 * verbosity is 2 or higher.
	 */
	if(1 < m_Verbose)
	{
		/* If -qq command option, do XML encode but don't actually print */
		if(2 > m_Quiet)
		{
			printf("\n===================================\n");
			printf("INFO: Message received\n");
		}
		printXMLMessage(pMessage);
	}

	return pMessage;
}


/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to send a message
 **
 ** Wrapper to send a message.
 **     - Print the message in XML if verbose level is at least 2
 **     - Send it using the LLRP_Conn_sendMessage()
 **     - Tattle on errors, if any
 **
 ** @param[in]  pSendMsg        Pointer to message to send
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong, already tattled
 **
 *****************************************************************************/

int
LLRPTestClient::sendMessage (
		CMessage *                    pSendMsg)
{
	CConnection *               pConn = m_pConnectionToReader;

	/*
	 * Print the XML text for the outbound message if
	 * verbosity is 2 or higher.
	 */
	if(1 < m_Verbose)
	{
		/* If -qq command option, do XML encode but don't actually print */
		if(2 > m_Quiet)
		{
			printf("\n===================================\n");
			printf("INFO: Sending\n");
		}
		printXMLMessage(pSendMsg);
	}

	/*
	 * If LLRP::CConnection::sendMessage() returns other than RC_OK
	 * then there was an error. In that case we try to print
	 * the error details.
	 */
	if(RC_OK != pConn->sendMessage(pSendMsg))
	{
		const CErrorDetails *   pError = pConn->getSendError();

		printf("ERROR: %s sendMessage failed, %s\n",
				pSendMsg->m_pType->m_pName,
				pError->m_pWhatStr ? pError->m_pWhatStr : "no reason given");

		if(NULL != pError->m_pRefType)
		{
			printf("ERROR: ... reference type %s\n",
					pError->m_pRefType->m_pName);
		}

		if(NULL != pError->m_pRefField)
		{
			printf("ERROR: ... reference field %s\n",
					pError->m_pRefField->m_pName);
		}

		return -1;
	}

	/*
	 * Victory
	 */
	return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Helper to print a message as XML text
 **
 ** Print a LLRP message as XML text
 **
 ** @param[in]  pMessage        Pointer to message to print
 **
 ** @return     void
 **
 *****************************************************************************/

void
LLRPTestClient::printXMLMessage (
		CMessage *                    pMessage)
{
	char                        aBuf[100*1024];

	/*
	 * Convert the message to an XML string.
	 * This fills the buffer with either the XML string
	 * or an error message. The return value could
	 * be checked.
	 */

	pMessage->toXMLString(aBuf, sizeof aBuf);

	/*
	 * Print the XML Text to the standard output.
	 * For characterization, command line option -qq
	 * prevents the XML text actually being printed and
	 * so the CPU utilization of XMLTextEncoder can be measured
	 * without the noise.
	 */
	if(2 > m_Quiet)
	{
		printf("%s", aBuf);
	}
}

LLRP::CROSpec*
LLRPTestClient::getBasicROSpec() {
	CROSpecStartTrigger *       pROSpecStartTrigger =
			new CROSpecStartTrigger();
	pROSpecStartTrigger->setROSpecStartTriggerType(
			ROSpecStartTriggerType_Null);

	CROSpecStopTrigger *        pROSpecStopTrigger = new CROSpecStopTrigger();
	pROSpecStopTrigger->setROSpecStopTriggerType(ROSpecStopTriggerType_Null);
	pROSpecStopTrigger->setDurationTriggerValue(0);     /* n/a */

	CROBoundarySpec *           pROBoundarySpec = new CROBoundarySpec();
	pROBoundarySpec->setROSpecStartTrigger(pROSpecStartTrigger);
	pROBoundarySpec->setROSpecStopTrigger(pROSpecStopTrigger);

	/*
	CTagReportContentSelector * pTagReportContentSelector =
			new CTagReportContentSelector();
	pTagReportContentSelector->setEnableROSpecID(FALSE);
	pTagReportContentSelector->setEnableSpecIndex(FALSE);
	pTagReportContentSelector->setEnableInventoryParameterSpecID(FALSE);
	pTagReportContentSelector->setEnableAntennaID(FALSE);
	pTagReportContentSelector->setEnableChannelIndex(FALSE);
	pTagReportContentSelector->setEnablePeakRSSI(FALSE);
	pTagReportContentSelector->setEnableFirstSeenTimestamp(FALSE);
	pTagReportContentSelector->setEnableLastSeenTimestamp(FALSE);
	pTagReportContentSelector->setEnableTagSeenCount(FALSE);
	pTagReportContentSelector->setEnableAccessSpecID(FALSE);

	CROReportSpec *             pROReportSpec = new CROReportSpec();
	pROReportSpec->setROReportTrigger(
			ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec);
	pROReportSpec->setN(1);         // Report every singulation
	pROReportSpec->setTagReportContentSelector(pTagReportContentSelector);
	 */

	CROSpec *                   pROSpec = new CROSpec();
	pROSpec->setROSpecID(123);
	pROSpec->setPriority(0);
	pROSpec->setCurrentState(ROSpecState_Disabled);
	pROSpec->setROBoundarySpec(pROBoundarySpec);
	//pROSpec->setROReportSpec(pROReportSpec);

	return pROSpec;
}

LLRP::CAISpec*
LLRPTestClient::getBasicAISpec() {

#if 1
	CAISpecStopTrigger *        pAISpecStopTrigger = new CAISpecStopTrigger();
	pAISpecStopTrigger->setAISpecStopTriggerType(AISpecStopTriggerType_Null);
	pAISpecStopTrigger->setDurationTrigger(0);
#else
	CTagObservationTrigger *    pTagObservationTrigger =
			new CTagObservationTrigger();
	pTagObservationTrigger->setTriggerType (
			TagObservationTriggerType_Upon_Seeing_No_More_New_Tags_For_Tms_Or_Timeout);
	pTagObservationTrigger->setNumberOfTags(0);         /* unlimited */
	pTagObservationTrigger->setNumberOfAttempts(0);     /* n/a */
	pTagObservationTrigger->setT(3500);                 /* 3.5 sec */
	pTagObservationTrigger->setTimeout(12500);          /* 12.5 sec */

	CAISpecStopTrigger *        pAISpecStopTrigger = new CAISpecStopTrigger();
	pAISpecStopTrigger->setAISpecStopTriggerType(
			AISpecStopTriggerType_Tag_Observation);
	pAISpecStopTrigger->setDurationTrigger(0);
	pAISpecStopTrigger->setTagObservationTrigger(pTagObservationTrigger);

	CC1G2SingulationControl *   pC1G2SingulationControl =
			new CC1G2SingulationControl();
	pC1G2SingulationControl->setSession(2);
	pC1G2SingulationControl->setTagPopulation(32);

	CC1G2InventoryCommand *     pC1G2InventoryCommand =
			new CC1G2InventoryCommand();
	pC1G2InventoryCommand->setC1G2SingulationControl(
			pC1G2SingulationControl);

	/*
	 * C1G2RFControl might be set below if a m_ModeIndex was
	 * given on the command line.
	 */
	CAntennaConfiguration *     pAntennaConfiguration =
			new CAntennaConfiguration();
	pAntennaConfiguration->addAirProtocolInventoryCommandSettings (
			pC1G2InventoryCommand);
#endif

	CInventoryParameterSpec *   pInventoryParameterSpec =
			new CInventoryParameterSpec();
	pInventoryParameterSpec->setInventoryParameterSpecID(1);
	pInventoryParameterSpec->setProtocolID(AirProtocols_EPCGlobalClass1Gen2);

	//pInventoryParameterSpec->addAntennaConfiguration (pAntennaConfiguration);

	llrp_u16v_t                 AntennaIDs = llrp_u16v_t(1);
	AntennaIDs.m_pValue[0] = 1;         /* All */

	CAISpec *                   pAISpec = new CAISpec();
	pAISpec->setAntennaIDs(AntennaIDs);
	pAISpec->setAISpecStopTrigger(pAISpecStopTrigger);
	pAISpec->addInventoryParameterSpec(pInventoryParameterSpec);

	return pAISpec;
}

LLRP::CAccessSpec*
LLRPTestClient::getBasicAccessSpec() {
	CAccessSpec *pAccessSpec = new CAccessSpec();
	pAccessSpec->setAccessSpecID(1);
	pAccessSpec->setAntennaID(0);
	pAccessSpec->setProtocolID(LLRP::AirProtocols_EPCGlobalClass1Gen2);
	pAccessSpec->setCurrentState(LLRP::AccessSpecState_Disabled);
	pAccessSpec->setROSpecID(0);
	CAccessSpecStopTrigger *pAccessSpecStopTrigger = new CAccessSpecStopTrigger();
	pAccessSpecStopTrigger->setAccessSpecStopTrigger(LLRP::AccessSpecStopTriggerType_Operation_Count);
	pAccessSpecStopTrigger->setOperationCountValue(1);
	pAccessSpec->setAccessSpecStopTrigger(pAccessSpecStopTrigger);

	return pAccessSpec;
}


LLRP::CC1G2TagSpec* LLRPTestClient::getTagSpecMatchAll() {
	CC1G2TagSpec *pC1G2TagSpec = new CC1G2TagSpec();
	CC1G2TargetTag *pC1G2TargetTag = new CC1G2TargetTag();
	pC1G2TargetTag->setMB(1);
	pC1G2TargetTag->setPointer(0);
	// length is not defined in LLRP 1.1 document.
	pC1G2TargetTag->setTagMask(llrp_u1v_t(0));
	pC1G2TargetTag->setTagData(llrp_u1v_t(0));
	pC1G2TargetTag->setMatch(1);
	pC1G2TagSpec->addC1G2TargetTag(pC1G2TargetTag);

	return pC1G2TagSpec;
}

LLRP::CC1G2Write* LLRPTestClient::getOpSpecWriteEPC() {
	CC1G2Write *pC1G2Write = new CC1G2Write();
	pC1G2Write->setOpSpecID(1);
	pC1G2Write->setMB(1);
	pC1G2Write->setWordPointer(1);
	llrp_u16v_t pWriteData(7);
	pWriteData.m_pValue[0] = 0x3000;
	for (int i = 1;i < 7;i++) {
		pWriteData.m_pValue[i] = 0x0000;
	}
	pC1G2Write->setWriteData(pWriteData);
	pC1G2Write->setAccessPassword(0);

	return pC1G2Write;
}

LLRP::CC1G2Read* LLRPTestClient::getOpSpecReadEPC() {
	CC1G2Read *pC1G2Read = new CC1G2Read();
	pC1G2Read->setOpSpecID(1);
	pC1G2Read->setMB(1);
	pC1G2Read->setWordPointer(0);
	pC1G2Read->setWordCount(0);
	pC1G2Read->setAccessPassword(0);

	return pC1G2Read;
}


LLRP::CInventoryParameterSpec* LLRPTestClient::getROSpecFilterMatchAll() {
	CInventoryParameterSpec *pInventoryParameterSpec = new CInventoryParameterSpec();
	pInventoryParameterSpec->setInventoryParameterSpecID(1);
	pInventoryParameterSpec->setProtocolID(LLRP::AirProtocols_EPCGlobalClass1Gen2);
	CAntennaConfiguration *pAntennaConfiguration = new CAntennaConfiguration();
	pAntennaConfiguration->setAntennaID(0);
	CC1G2InventoryCommand *pC1G2InventoryCommand = new CC1G2InventoryCommand();
	pC1G2InventoryCommand->setTagInventoryStateAware(0);
	CC1G2Filter *pC1G2Filter = new CC1G2Filter();
	CC1G2TagInventoryMask *pC1G2TagInventoryMask = new CC1G2TagInventoryMask();
	pC1G2TagInventoryMask->setMB(1);
	pC1G2TagInventoryMask->setPointer(0);
	// length is not specified in LLRP 1.1 document
	pC1G2TagInventoryMask->setTagMask(llrp_u1v_t(0));
	pC1G2Filter->setC1G2TagInventoryMask(pC1G2TagInventoryMask);
	pC1G2Filter->setT(LLRP::C1G2TruncateAction_Unspecified);
	CC1G2TagInventoryStateUnawareFilterAction *pC1G2TagInventoryStateUnawareFilterAction = new CC1G2TagInventoryStateUnawareFilterAction();
	pC1G2TagInventoryStateUnawareFilterAction->setAction(LLRP::C1G2StateUnawareAction_Select_Unselect);
	pC1G2Filter->setC1G2TagInventoryStateUnawareFilterAction(pC1G2TagInventoryStateUnawareFilterAction);
	pC1G2InventoryCommand->addC1G2Filter(pC1G2Filter);
	pAntennaConfiguration->addAirProtocolInventoryCommandSettings((CParameter *) pC1G2InventoryCommand);
	pInventoryParameterSpec->addAntennaConfiguration(pAntennaConfiguration);

	return pInventoryParameterSpec;
}


int
LLRPTestClient::checkLLRPConfigurationStateValue(llrp_u32_t *pLLRPConfigurationStateValue, const char *testName) {
	CGET_READER_CONFIG *        pCmd;
	CMessage *                  pRspMsg;
	CGET_READER_CONFIG_RESPONSE *pRsp;

	pCmd = new CGET_READER_CONFIG();
	pCmd->setMessageID(121);
	pCmd->setRequestedData((enum EGetReaderConfigRequestedData) 7);

	pRspMsg = this->transact(pCmd);

	delete pCmd;

	if(NULL == pRspMsg)
	{
		return -1;
	}

	pRsp = (CGET_READER_CONFIG_RESPONSE *) pRspMsg;

	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), testName))
	{
		delete pRspMsg;
		return -1;
	}

	printf ("INFO: LLRPConfigurationStateValue = %d\n", pRsp->getLLRPConfigurationStateValue()->getLLRPConfigurationStateValue());

	if (*pLLRPConfigurationStateValue == pRsp->getLLRPConfigurationStateValue()->getLLRPConfigurationStateValue()) {
		delete pRspMsg;
		return -1;
	}

	*pLLRPConfigurationStateValue = pRsp->getLLRPConfigurationStateValue()->getLLRPConfigurationStateValue();

	delete pRspMsg;


	return 0;
}

int LLRPTestClient::startROSpec(llrp_u32_t ROSpecID, const char *testName) {
	CSTART_ROSPEC *       		pCmd;
	CMessage *                  pRspMsg;
	CSTART_ROSPEC_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CSTART_ROSPEC();
	pCmd->setMessageID(133);
	pCmd->setROSpecID(ROSpecID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	if(&CSTART_ROSPEC_RESPONSE::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CSTART_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), "TCR_R6_Step8"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: %s failed\n", testName);
	delete pRspMsg;
	return -1;

}

int LLRPTestClient::stopROSpec(llrp_u32_t ROSpecID, const char *testName) {
	CSTOP_ROSPEC *       		pCmd;
	CMessage *                  pRspMsg;
	CSTOP_ROSPEC_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CSTOP_ROSPEC();
	pCmd->setMessageID(133);
	pCmd->setROSpecID(ROSpecID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	if(&CSTOP_ROSPEC_RESPONSE::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CSTOP_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), testName))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;


	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: %s failed\n", testName);
	delete pRspMsg;
	return -1;

}


int LLRPTestClient::deleteROSpec(llrp_u32_t ROSpecID, const char *testName) {
	CDELETE_ROSPEC *       		pCmd;
	CMessage *                  pRspMsg;
	CDELETE_ROSPEC_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CDELETE_ROSPEC();
	pCmd->setMessageID(133);
	pCmd->setROSpecID(ROSpecID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	if(&CDELETE_ROSPEC_RESPONSE::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CDELETE_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), testName))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: %s failed\n", testName);
	delete pRspMsg;
	return -1;

}


int LLRPTestClient::disableROSpec(llrp_u32_t ROSpecID, const char *testName) {
	CDISABLE_ROSPEC *       		pCmd;
	CMessage *                  pRspMsg;
	CDISABLE_ROSPEC_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CDISABLE_ROSPEC();
	pCmd->setMessageID(133);
	pCmd->setROSpecID(ROSpecID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	if(&CDISABLE_ROSPEC_RESPONSE::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CDISABLE_ROSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), testName))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: %s failed\n", testName);
	delete pRspMsg;
	return -1;

}

CREADER_EVENT_NOTIFICATION* LLRPTestClient::recvNotification(int nMaxMs, const char *testName) {
	CMessage *                  pRspMsg;
	CREADER_EVENT_NOTIFICATION *pRsp;
	/*
	 * Expect the notification within 10 seconds.
	 * It is suppose to be the very first message sent.
	 */
	pRsp = new CREADER_EVENT_NOTIFICATION();
	pRspMsg = this->m_pConnectionToReader->recvResponse(nMaxMs,pRsp->m_pType,0);
	delete pRsp;

	/*
	 * recvMessage() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* recvMessage already tattled */
		goto fail;
	}

	/*
	 * Check to make sure the message is of the right type.
	 * The type label (pointer) in the message should be
	 * the type descriptor for READER_EVENT_NOTIFICATION.
	 */
	if(&CREADER_EVENT_NOTIFICATION::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	/*
	 * Now that we are sure it is a READER_EVENT_NOTIFICATION,
	 * traverse to the ReaderEventNotificationData parameter.
	 */
	pRsp = (CREADER_EVENT_NOTIFICATION *) pRspMsg;

	printf ("INFO: %s: READER_EVENT_NOTIFICATION is received\n", testName);

	return pRsp;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: %s, recvEventNotification failed\n", testName);
	delete pRspMsg;
	return NULL;
}

CRO_ACCESS_REPORT* LLRPTestClient::getROAccessReport(int nMaxMs, const char *testName) {
	CMessage *                  pRspMsg = NULL;
	CRO_ACCESS_REPORT *			pReport;

	pReport = new CRO_ACCESS_REPORT();
	pRspMsg = this->m_pConnectionToReader->recvResponse(nMaxMs,pReport->m_pType,0);
	delete pReport;

	if(NULL == pRspMsg)
	{
		/* recvMessage already tattled */
		goto fail;
	}

	if(&CRO_ACCESS_REPORT::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	pReport = (CRO_ACCESS_REPORT *) pRspMsg;

	if(this->m_Verbose)
	{
		printf("INFO: RO_ACCESS_REPORT is received\n");
	}

	return pReport;

	fail:
	printf("ERROR: %s, failed to receive RO_ACCESS_REPORT\n", testName);
	if (pRspMsg != NULL)
		delete pRspMsg;
	return NULL;

}


uint64_t LLRPTestClient::getCurrentTimeMilliseconds() {
	time_t timer;
	time(&timer);
	uint64_t time_milliseconds = timer * 1000;

	return time_milliseconds;
}

int LLRPTestClient::sendCloseConnectionWithCustomCommand(
		enum ECLOSECONNECTION_CUSTOM_COMMAND pCommand) {
	CCLOSE_CONNECTION *        pCmd;
	CMessage *                  pRspMsg;
	CCLOSE_CONNECTION_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CCLOSE_CONNECTION();
	// If the message ID of close connection is 979797, then reader shuts down.
	llrp_u32_t msgID = static_cast<llrp_u32_t>(pCommand);
	pCmd->setMessageID(msgID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CCLOSE_CONNECTION_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), "getLLRPConfigurationStateValue"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Victory.
	 */
	return 0;
}

int LLRPTestClient::sendCloseConnectionWithCustomCommand(
		int GPIPortNum, int GPIEvent) {
	CCLOSE_CONNECTION *        pCmd;
	CMessage *                  pRspMsg;
	CCLOSE_CONNECTION_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CCLOSE_CONNECTION();
	// If the received message ID is bigger than 1000000, it is GPI event.
	// For example, in 10000$%, $ is port number and % is event.
	llrp_u32_t msgID = static_cast<llrp_u32_t>(1000000 + GPIPortNum * 10 + GPIEvent);
	pCmd->setMessageID(msgID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CCLOSE_CONNECTION_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), "getLLRPConfigurationStateValue"))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	/*
	 * Victory.
	 */
	return 0;
}

std::size_t LLRPTestClient::getTagReportHash(LLRP::CTagReportData* pData) {
	LLRP::llrp_u8_t *mk = &(((LLRP::CEPC_96 *) pData->getEPCParameter())->getEPC().m_aValue[0]);
	std::size_t seed = 0;
	for (int i = 0;i < 12;i++) {
		boost::hash_combine(seed, mk[i]);
	}
	return seed;
}

int LLRPTestClient::enableAccessSpec(llrp_u32_t AccessSpecID,
		const char* testName) {
	CENABLE_ACCESSSPEC *       		pCmd;
	CMessage *                  pRspMsg;
	CENABLE_ACCESSSPEC_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CENABLE_ACCESSSPEC();
	pCmd->setMessageID(133);
	pCmd->setAccessSpecID(AccessSpecID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	if(&CENABLE_ACCESSSPEC_RESPONSE::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CENABLE_ACCESSSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), testName))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: %s failed\n", testName);
	delete pRspMsg;
	return -1;
}

int LLRPTestClient::disableAccessSpec(llrp_u32_t AccessSpecID,
		const char* testName) {
	CDISABLE_ACCESSSPEC *       		pCmd;
	CMessage *                  pRspMsg;
	CDISABLE_ACCESSSPEC_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CDISABLE_ACCESSSPEC();
	pCmd->setMessageID(133);
	pCmd->setAccessSpecID(AccessSpecID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	if(&CDISABLE_ACCESSSPEC_RESPONSE::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CDISABLE_ACCESSSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), testName))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: %s failed\n", testName);
	delete pRspMsg;
	return -1;
}

int LLRPTestClient::deleteAccessSpec(llrp_u32_t AccessSpecID,
		const char* testName) {
	CDELETE_ACCESSSPEC *       		pCmd;
	CMessage *                  pRspMsg;
	CDELETE_ACCESSSPEC_RESPONSE *pRsp;

	/*
	 * Compose the command message
	 */
	pCmd = new CDELETE_ACCESSSPEC();
	pCmd->setMessageID(133);
	pCmd->setAccessSpecID(AccessSpecID);

	/*
	 * Send the message, expect the response of certain type
	 */
	pRspMsg = this->transact(pCmd);

	/*
	 * Done with the command message
	 */
	delete pCmd;

	/*
	 * transact() returns NULL if something went wrong.
	 */
	if(NULL == pRspMsg)
	{
		/* transact already tattled */
		return -1;
	}

	if(&CDELETE_ACCESSSPEC_RESPONSE::s_typeDescriptor != pRspMsg->m_pType)
	{
		goto fail;
	}

	/*
	 * Cast to a GET_READER_CONFIG_RESPONSE message.
	 */
	pRsp = (CDELETE_ACCESSSPEC_RESPONSE *) pRspMsg;

	/*
	 * Check the LLRPStatus parameter.
	 */
	if(0 != this->checkLLRPStatus(pRsp->getLLRPStatus(), testName))
	{
		/* checkLLRPStatus already tattled */
		delete pRspMsg;
		return -1;
	}

	/*
	 * Done with the response message.
	 */
	delete pRspMsg;

	return 0;

	fail:
	/*
	 * Something went wrong. Tattle. Clean up. Return error.
	 */
	printf("ERROR: %s failed\n", testName);
	delete pRspMsg;
	return -1;
}

int LLRPTestClient::compareCElems(LLRP::CElement* pElem1,
		LLRP::CElement* pElem2) {
	char *pBuf1 = new char[100*1024];
	char *pBuf2 = new char[100*1024];
	LLRP::toXMLString(pElem1, pBuf1, 100*1024);
	LLRP::toXMLString(pElem2, pBuf2, 100*1024);
	int cmpResult = strcmp(pBuf1, pBuf2);
	//int cmpResult = memcmp(pBuf1, pBuf2, 100*1024);
	printf ("pElem1: \n%s\npElem2: \n%s\nResult = %s", pBuf1, pBuf2, (cmpResult == 0) ? "Same\n" : "Different\n");

	delete pBuf1;
	delete pBuf2;
	return cmpResult;
}
