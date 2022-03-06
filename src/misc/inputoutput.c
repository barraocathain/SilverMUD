// inputoutput.c: Implementation of input output library for SilverMUD.
// Barry Kane, 2022.
#include <stdio.h>
#include "inputoutput.h"
#include <gnutls/gnutls.h>

// Sends a message to a given TLS session, wraps the calls to gnutls_write:
void messageSend(gnutls_session_t receivingSession, userMessage * messageToSend)
{
	int returnValue = 0;
	do
	{
		returnValue = gnutls_record_send(receivingSession, messageToSend->senderName, sizeof(((userMessage*)0)->senderName));
	}  while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);
	do
	{
		returnValue = gnutls_record_send(receivingSession, messageToSend->messageContent, sizeof(((userMessage*)0)->messageContent));
	} while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);
}

// Recieves a message from a given TLS session, wraps the calls to gnutls_read:
void messageReceive(gnutls_session_t receiveFromSession, userMessage * receiveToMessage)
{
	int returnValue = 0;
	do
	{
		returnValue = gnutls_record_recv(receiveFromSession, receiveToMessage->senderName, sizeof(((userMessage*)0)->senderName));
	}  while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);
	do
	{
		returnValue = gnutls_record_recv(receiveFromSession, receiveToMessage->messageContent, sizeof(((userMessage*)0)->messageContent));
	} while (returnValue == GNUTLS_E_AGAIN || returnValue == GNUTLS_E_INTERRUPTED);
}
