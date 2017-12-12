#include <Windows.h>
#include <string>
#include "SharedMemoryBuffer.h"
#include "ComLibHeaders.h"
#include <mutex>
#include <tchar.h>
#include <vector>
#define MB 1000000
typedef SharedMemoryBuffer SharedMemory;
class ComLib
{
private:
//+-+-+- Buffers -+-+-+
	SharedMemory ringBuffer;
	SharedMemory ringBufferData;

//+-+-+- Mutex -+-+-+
	HANDLE hnd_Mutex;

//+-+-+- POD members -+-+-+
	size_t ringBufferSize;
	size_t head;
	size_t tail;

//+-+-+- Private functions -+-+-+
	void UpdateRBD(size_t value);
	size_t GetTail();
	size_t GetHead();


	PVOID pRingBuffer; //?
public:
	enum TYPE{ PRODUCER, CONSUMER }type;
	enum MSG_TYPE{ NORMAL, DUMMY };
	struct Header
	{
		size_t msgId;
		size_t msgSeq;
		size_t msgLength;
	};

	// create a ComLib
	ComLib(const std::string& secret, const size_t& buffSize, TYPE type);

	// init and check status
	bool connect();
	bool isConnected();

	// returns "true" if data was sent successfully.
	// false if for any reason the data could not be sent.
	bool send(const void * msg, const size_t length);


	/*
		returns: "true" if a message was received.
				 "false" if there is nothing to read.
		"msg" is expected to have enough space, use "nextSize" to
		check this and allocate if needed, but outside ComLib.
		length should indicate the length of the data read.
		Should never return DUMMY messages.
	*/
	bool recv(char * msg, size_t & length);
	bool recv1();

	/* return the length of the next message */
	size_t nextSize();

	/* disconnect and destroy all resources */
	~ComLib();
};

