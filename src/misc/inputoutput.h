// inputoutput.h: Header file contatning function prototypes and datastructures
// for dealing with input and output.
// Barry Kane, 2022.
#ifndef INPUTOUTPUt_H
#define INPUTOUTPUT_H
#include <gnutls/gnutls.h>

// A message datastructure containing a user/character name and the content:
typedef struct userMessage
{
	char senderName[32];
	char messageContent[2048];
} userMessage;

// Sends a message to a given TLS session, wraps the calls to gnutls_write:
void messageSend(gnutls_session_t receivingSession, userMessage * messageToSend);

// Recieves a message from a given TLS session, wraps the calls to gnutls_read:
void messageReceive(gnutls_session_t receiveFromSession, userMessage * receiveToMessage);

#endif
