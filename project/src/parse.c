#include "parse.h"
// #define PARSEDEBUG

/**
 * Given a char buffer returns the parsed request headers
 */
Request *parse(char *buffer, int size, int socketFd)
{
	// Differant states in the state machine
	//   printf("%s",buffer);
	//   printf("state machine\n");
	enum
	{
		STATE_START = 0,
		STATE_CR,
		STATE_CRLF,
		STATE_CRLFCR,
		STATE_CRLFCRLF
	};

	int i = 0, state;
	size_t offset = 0;
	char ch;
	char buf[8192];
	memset(buf, 0, 8192);
	int n = 0;
	while (buffer[i] == '\r' || buffer[i] == '\n')
	{
		i++;
		n++;
	}
	state = STATE_START;
	while (state != STATE_CRLFCRLF)
	{
		char expected = 0;

		if (i == size)
			break;

		ch = buffer[i++];
		buf[offset++] = ch;
		switch (state)
		{
		case STATE_START:
		case STATE_CRLF:
			expected = '\r';
			break;
		case STATE_CR:
		case STATE_CRLFCR:
			expected = '\n';
			break;
		default:
			state = STATE_START;
			continue;
		}

		if (ch == expected)
			state++;
		else
			state = STATE_START;
	}

	// Valid End State
	if (state == STATE_CRLFCRLF)
	{
#ifdef PARSEDEBUG
		printf("==========TRY TO PARSE==========\n");
		printf("----------Parsing MSG\n%s\n", buf);
#endif
		Request *request = (Request *)malloc(sizeof(Request));
		request->header_count = 0;
		// TODO You will need to handle resizing this in parser.y
		request->headers = (Request_header *)malloc(sizeof(Request_header) * 100);
#ifdef PARSEDEBUG
		printf("==========Parsing buf==========\n%s\n", buf);
#endif
		set_parsing_options(buf, i - n, request);
		// 输出buf
		if (yyparse() == SUCCESS)
		{
			yyrestart(NULL);
			return request;
		}
	}
	// TODO Handle Malformed Requests
	printf("Parsing Failed\n");
	return NULL;
}
