#include "ComLib.h"
#define RETURN_UNLOCK_FALSE(x) {ReleaseMutex(x); return false;}
#define RETURN_UNLOCK_TRUE(x) {ReleaseMutex(x); return true;}

#define RETURN_SAFE_FALSE RETURN_UNLOCK_FALSE(hnd_Mutex)
#define RETURN_SAFE_TRUE RETURN_UNLOCK_TRUE(hnd_Mutex)

// -------------------- [ Utility functions ] --------------------

bool MessageFits(size_t head, size_t tail, size_t bufferSize, size_t messageSize)
{
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Check if message fits in remaining memory, between head and EOB
	//and between head and tail.
	if (bufferSize - head < messageSize)
		return false;
	if (tail > head && (tail - head) - 1 < messageSize)
		return false;
	else return true;
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
}

ComLib::Header CreateHeader(size_t type, size_t length /*incl null-T*/)
{
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Create and return a header based on passed values
	ComLib::Header header;
	header.msgId = type;
	header.msgLength = length;
	header.msgSeq = 1;			//Always 1, since it is not used.
	return header;
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
}

void ComLib::UpdateRBD(size_t value)
{
	///+-+-+-+-+-+-+-+-+-+-+-+
	//Check who wants to update; producer and consumer can only update
	//their respective value (head/tail)
	if (this->type == ComLib::TYPE::PRODUCER)
	{
		CopyMemory((PVOID)ringBufferData.GetBuffer(), &value, sizeof(size_t)); //First slot is head
	}
	///+-+-+-+-+-+-+-+-+-+-+-+
	else
	{
		CopyMemory((PVOID)(ringBufferData.GetBuffer() + sizeof(size_t)), &value, sizeof(size_t)); //Second slot is tail
	}
	///+-+-+-+-+-+-+-+-+-+-+-+
}

size_t ComLib::GetTail()
{
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Retrieve tail from RBD
	size_t r;
	CopyMemory(&r, (PVOID)(ringBufferData.GetBuffer() + sizeof(size_t)), sizeof(size_t));
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	return r;
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
}

size_t ComLib::GetHead()
{
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Retrieve head from RBD
	size_t r;
	CopyMemory(&r, (PVOID)ringBufferData.GetBuffer(), sizeof(size_t));
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	return r;
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
}

// ---------------------------------------------------------------

ComLib::ComLib(const std::string& secret, const size_t& buffSize, TYPE type)
{
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Initialize POD members
	this->type = type;
	this->head = this->tail = 0;
	this->ringBufferSize = buffSize;
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Convert string to wide string
	std::wstring widestr = std::wstring(secret.begin(), secret.end());
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Initialize ring buffer
	this->ringBuffer.Init(secret, buffSize);
	this->pRingBuffer = (PVOID)ringBuffer.GetBuffer(); //Not used
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//ringBufferData holds space for head and tail (size_t)
	//File Map is called RBD (RingBufferData)
	this->ringBufferData.Init("RBD", sizeof(size_t) * 2);
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Initialize RBD and Create mutex, if we are the producer
	{
		if (this->type == PRODUCER)
		{
			this->hnd_Mutex = CreateMutex(NULL, FALSE, _T("comlibmtx"));
			CopyMemory(this->ringBufferData.GetBuffer(), &this->head, sizeof(size_t));
			CopyMemory(this->ringBufferData.GetBuffer() + sizeof(size_t), &this->tail, sizeof(size_t));
		}
		///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		//If we are not the producer, open the mutex instead
		else
			this->hnd_Mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T("comlibmtx"));
		///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	}
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

}

void ComLib::Init(std::unordered_map<string, Model*>& modelMap, Shader* vs, Shader* ps)
{
	m_mModels  = &modelMap;
	m_vs = vs;
	m_ps = ps;
}

ComLib::~ComLib()
{}

bool ComLib::send(const void * msg, const size_t length)
{
	///+-+-+-+-+-+-+-+-+-+-+-+
	//length with header
	size_t _length = length + sizeof(ComLib::Header);
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Acquire mutex
	WaitForSingleObject(hnd_Mutex, INFINITE);
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Get latest tail
	this->tail = this->GetTail();
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//determine if tail is behind head in memory
	bool tailIsBehind = (tail < head);
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Determine space left until EndOfBuffer
	size_t sizeToEOB = ringBufferSize - head;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Determine space left until tail
	size_t sizeToTail;
	tailIsBehind ? sizeToTail = (sizeToEOB + tail - 1) % ringBufferSize : sizeToTail = tail - head - 1;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//If tail == head, remaining memory is size of buffer minus 1 
	//(head == tail-1 means full, in order to distinguish between empty and full)
	if (tail == head)
	{
		sizeToTail = ringBufferSize - 1;
		tailIsBehind = true;
	}
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Check if message fits (in remaining memory and between head and tail
	//if it fits, create and send header, then send message and return true
	if (MessageFits(head, tail, ringBufferSize, _length))
	{

		auto hdr = CreateHeader(1, length);
		char* cpMsg = (char*)msg;
		char* pByte = (char*)this->pRingBuffer + head;

		//Send header and advance head
		this->ringBuffer.Send(pByte, (PVOID)&hdr, sizeof(hdr));
		pByte += sizeof(hdr);

		//send message
		this->ringBuffer.Send(pByte, cpMsg, length);
		head = (head + _length) % this->ringBufferSize;
		UpdateRBD(head);
		RETURN_SAFE_TRUE;
	}
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//if message doesnt fit and tail is in front of head, we cant pad memory;
	//return and wait for tail to advance
	else if (tail > head)
	{
		RETURN_SAFE_FALSE;
	}
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//we can not fit the message in the remaining memory. Pad and set 
	//head to 0. (Make sure tail is > 0, since that would imply that 
	//the buffer is empty, when it is in fact full. Thus we check for
	//tail < 0.
	else if (tail > 0)
	{
		// *** PAD END ***
		///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		//if we can fit a header in the remaining memory, create a dummy
		//message. If not, just set remaining memory to '\0' and set head = 0 
		//and return false.
		if (sizeToEOB >= sizeof(Header))
		{
			auto hdr = CreateHeader(0, ringBufferSize - head);
			this->ringBuffer.Send(ringBuffer.GetBuffer() + head, (PVOID)&hdr, sizeof(hdr));
		}
		memset(ringBuffer.GetBuffer() + head, 0, ringBufferSize - head);
		head = 0;
		this->UpdateRBD(head);
		RETURN_SAFE_FALSE;
	}
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//This should be redundant. Keeping it just in case
	else if (tailIsBehind && sizeToEOB < sizeof(Header) && tail > 0)
	{
		head = 0;
		this->UpdateRBD(head);
		RETURN_SAFE_FALSE;
	}
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

	RETURN_SAFE_FALSE;

}

bool ComLib::recv(char * msg, size_t & length)
{
	///+-+-+-+-+-+-+-+-+-+-+-+
	//Acquire mutex
	WaitForSingleObject(hnd_Mutex, INFINITE);
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Update head
	this->head = GetHead();
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Create pointer from where we will start recieving 
	char* pByte = (char*)this->pRingBuffer + tail;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Determine if head is in front of tail
	bool headIsInFront = (head >= tail);
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Determine how much memory is left in the buffer
	size_t sizeToEOB = ringBufferSize - tail;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Determine how much memory is left until head
	size_t sizeToHead;
	headIsInFront ? sizeToHead = head - tail : sizeToHead = ringBufferSize - tail + head;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Recieve header if there is space
	//for it.
	Header hdr;
	if (sizeToHead >= sizeof(Header) && sizeToEOB >= sizeof(Header))
	{
		CopyMemory(&hdr, pByte, sizeof(Header));
		pByte += sizeof(Header);
	}
	///+-+-+-+-+-+-+-+-+-+-+-+
	//If the remaining memory cant hold a Header, 
	//roll back to 0.
	else if (sizeToEOB < sizeof(Header))
	{
		//Make sure head is not in front (it should never be, however)
		if (!headIsInFront)
		{
			tail = 0;
			UpdateRBD(tail);
			RETURN_SAFE_FALSE;
		}
		else
		{
			RETURN_SAFE_FALSE;
		}
	}
	///+-+-+-+-+-+-+-+-+-+-+-+
	else RETURN_SAFE_FALSE;
	///+-+-+-+-+-+-+-+-+-+-+-+
	
	///+-+-+-+-+-+-+-+-+-+-+-+
	//If msgId == 1, the message is a normal message. 
	//Recieve.
	if (hdr.msgId == 1)
	{
		CopyMemory(msg, pByte, hdr.msgLength);

		tail = (tail + hdr.msgLength + sizeof(Header)) % ringBufferSize;
		UpdateRBD(tail);
		length = hdr.msgLength;
		RETURN_SAFE_TRUE;
	}
	///+-+-+-+-+-+-+-+-+-+-+-+
	//If not, message is a dummy message (which means send() function
	//padded the memory). Roll back to 0.
	else
	{
		tail = 0;
		UpdateRBD(tail);
		RETURN_SAFE_FALSE;
	}
	///+-+-+-+-+-+-+-+-+-+-+-+

	RETURN_SAFE_FALSE;
}

bool ComLib::recv1(string& mdlName, vector<Vertex_pos3nor3uv2>& mdlVerts, XMMATRIX* matrix, sEditedCameraTransform& camStruct, MessageType& type)
{
	///+-+-+-+-+-+-+-+-+-+-+-+
	//Acquire mutex
	WaitForSingleObject(hnd_Mutex, INFINITE);
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Update head
	this->head = GetHead();
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Create pointer from where we will start recieving 
	char* pByte = (char*)this->pRingBuffer + tail;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Determine if head is in front of tail
	bool headIsInFront = (head >= tail);
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Determine how much memory is left in the buffer
	size_t sizeToEOB = ringBufferSize - tail;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Determine how much memory is left until head
	size_t sizeToHead;
	headIsInFront ? sizeToHead = head - tail : sizeToHead = ringBufferSize - tail + head;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//Recieve header if there is space
	//for it.
	Header hdr;
	if (sizeToHead >= sizeof(Header) && sizeToEOB >= sizeof(Header))
	{
		CopyMemory(&hdr, pByte, sizeof(Header));
		pByte += sizeof(Header);
	}
	///+-+-+-+-+-+-+-+-+-+-+-+
	//If the remaining memory cant hold a Header, 
	//roll back to 0.
	else if (sizeToEOB < sizeof(Header))
	{
		//Make sure head is not in front (it should never be, however)
		if (!headIsInFront)
		{
			tail = 0;
			UpdateRBD(tail);
			RETURN_SAFE_FALSE;
		}
		else
		{
			RETURN_SAFE_FALSE;
		}
	}
	///+-+-+-+-+-+-+-+-+-+-+-+
	else RETURN_SAFE_FALSE;
	///+-+-+-+-+-+-+-+-+-+-+-+

	///+-+-+-+-+-+-+-+-+-+-+-+
	//If msgId == 1, the message is a normal message. 
	//Recieve.
	if (hdr.msgId == 1)
	{
		//Original attempt
		{
			sPrimaryHeader primary = {};
			std::vector<Vertex_pos3nor3uv2> verts;
			CopyMemory(&primary, pByte, sizeof(sPrimaryHeader));
			pByte += sizeof(sPrimaryHeader);

			if		(primary.type == MessageType::NEW_MESH)
			{
				//OutputDebugStringA("Recieving NEW_MESH\n");
				type = NEW_MESH;

				sNewMeshHeader secondary = {};

				CopyMemory(&secondary, pByte, sizeof(sNewMeshHeader));
				pByte += sizeof(sNewMeshHeader);

				char* name = new char[secondary.nameLength + 1];
				memset(name, '\0', secondary.nameLength + 1);
				CopyMemory(name, pByte, sizeof(char) * secondary.nameLength);
				pByte += sizeof(char) * secondary.nameLength;

				verts.resize(secondary.vertCount);
				CopyMemory(verts.data(), pByte, sizeof(Vertex_pos3nor3uv2) * secondary.vertCount);
				pByte += sizeof(Vertex_pos3nor3uv2) * secondary.vertCount;

				mdlName = name;
				mdlVerts = verts;

				//UINT vertIndex = 0;
				//std::cout << name << "\t " << std::endl;
				//for (int i = 0; i < secondary.vertCount / 3; i++)
				//{
				//	//std::cout << "\tFace: " << i << std::endl;
				//	//std::cout << verts[vertIndex].x << ", " << verts[vertIndex].y << ", " << verts[vertIndex].z << std::endl;
				//	//vertIndex++;
				//	//std::cout << verts[vertIndex].x << ", " << verts[vertIndex].y << ", " << verts[vertIndex].z << std::endl;
				//	//vertIndex++;
				//	//std::cout << verts[vertIndex].x << ", " << verts[vertIndex].y << ", " << verts[vertIndex].z << std::endl;
				//	//vertIndex++;
				//}

				*matrix = DirectX::XMLoadFloat4x4(&secondary.worldTransform);

				tail = (tail + hdr.msgLength + sizeof(Header)) % ringBufferSize;
				UpdateRBD(tail);

				RETURN_SAFE_TRUE;
			}
			else if (primary.type == MessageType::MESH_REMOVED)
			{
				type = MESH_REMOVED;

				sMeshRemoved secondary = {};

				CopyMemory(&secondary, pByte, sizeof(secondary));
				pByte += sizeof(secondary);

				char* name = new char[secondary.nameLength + 1];
				memset(name, '\0', secondary.nameLength + 1);
				CopyMemory(name, pByte, sizeof(char) * secondary.nameLength);
				pByte += sizeof(char) * secondary.nameLength;

				mdlName = name;

				tail = (tail + hdr.msgLength + sizeof(Header)) % ringBufferSize;
				UpdateRBD(tail);

				RETURN_SAFE_TRUE;
			}
			else if (primary.type == MessageType::EDITED_MESH_TOPOLOGY)
			{
				type = EDITED_MESH_TOPOLOGY;

				sEditedTopology secondary = {};

				CopyMemory(&secondary, pByte, sizeof(secondary));
				pByte += sizeof(secondary);

				char* name = new char[secondary.nameLength + 1];
				memset(name, '\0', secondary.nameLength + 1);
				CopyMemory(name, pByte, sizeof(char) * secondary.nameLength);
				pByte += sizeof(char) * secondary.nameLength;

				verts.resize(secondary.vertCount);
				CopyMemory(verts.data(), pByte, sizeof(Vertex_pos3nor3uv2) * secondary.vertCount);
				pByte += sizeof(Vertex_pos3nor3uv2) * secondary.vertCount;

				mdlName = name;
				mdlVerts = verts;

				tail = (tail + hdr.msgLength + sizeof(Header)) % ringBufferSize;
				UpdateRBD(tail);

				RETURN_SAFE_TRUE;
			}
			else if (primary.type == MessageType::EDITED_MESH_TRANSFORM)
			{
				type = EDITED_MESH_TRANSFORM;

				sEditedMeshTransform secondary = {};

				CopyMemory(&secondary, pByte, sizeof(sEditedMeshTransform));
				pByte += sizeof(sEditedMeshTransform);

				char* name = new char[secondary.nameLength + 1];
				memset(name, '\0', secondary.nameLength + 1);
				CopyMemory(name, pByte, sizeof(char) * secondary.nameLength);
				pByte += sizeof(char) * secondary.nameLength;
				mdlName = name;
				*matrix = DirectX::XMLoadFloat4x4(&secondary.worldTransform);

				tail = (tail + hdr.msgLength + sizeof(Header)) % ringBufferSize;
				UpdateRBD(tail);

				RETURN_SAFE_TRUE;
			}
			else if (primary.type == MessageType::EDITED_CAMERA_TRANSFORM)
			{
				OutputDebugStringA("Recieving Camera transform\n");

				type = EDITED_CAMERA_TRANSFORM;

				sEditedCameraTransform secondary = {};

				CopyMemory(&secondary, pByte, sizeof(sEditedCameraTransform));
				pByte += sizeof(sEditedCameraTransform);

				camStruct = secondary;

				tail = (tail + hdr.msgLength + sizeof(Header)) % ringBufferSize;
				UpdateRBD(tail);

				RETURN_SAFE_TRUE;
			}
		}

	}
	///+-+-+-+-+-+-+-+-+-+-+-+
	//If not, message is a dummy message (which means send() function
	//padded the memory). Roll back to 0.
	else
	{
		tail = 0;
		UpdateRBD(tail);
		RETURN_SAFE_FALSE;
	}
	///+-+-+-+-+-+-+-+-+-+-+-+

	RETURN_SAFE_FALSE;
}

bool ComLib::connect()
{
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//nah
	return false;
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
}

bool ComLib::isConnected()
{
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//Check if buffers are initialized
	return (ringBuffer.GetBuffer() != nullptr && ringBufferData.GetBuffer() != nullptr);
	///+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
}

void ComLib::get(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Buffer* materialBuffer, Camera* camera, float w, float h)
{
	string name;
	vector<Vertex_pos3nor3uv2> verts;
	sEditedCameraTransform camStruct = {};
	XMMATRIX matrix;
	MessageType type;
	if (recv1(name, verts, &matrix, camStruct, type))
	{
		//check if model exists
		
		bool found = false;
		auto condition = [&](Model* model) { found = (*model == name); };
		std::for_each(m_mModels->begin(), m_mModels->end(), condition);

		if (type == MessageType::NEW_MESH)
		{
			Model* model = new Model(device, deviceContext, materialBuffer);
			model->LoadFromComLib(name.c_str(), verts);
			model->SetShaders(m_vs, m_ps, nullptr);
			model->SetWorldMatrix(matrix);
			m_mModels->insert({ name, model });
		}
		else if (type == MessageType::MESH_REMOVED)
		{
			delete m_mModels->at(name);
			m_mModels->erase(name);
		}
		else if (type == MessageType::EDITED_MESH_TRANSFORM)
		{
			m_mModels->at(name)->SetWorldMatrix(matrix);
		}
		else if (type == MessageType::EDITED_CAMERA_TRANSFORM)
		{
			XMVECTOR camPosition, camTarget, camUp;
			camPosition = XMVectorSet(camStruct.posX, camStruct.posY, camStruct.posZ, 0.0f);
			camTarget   = XMVectorSet(camStruct.tarX, camStruct.tarY, camStruct.tarZ, 0.0f);
			camUp       = XMVectorSet(camStruct.upX, camStruct.upY, camStruct.upZ, 0.0f);
			//Set the View matrix
			XMMATRIX camView = DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp);

			XMMATRIX camProjection;
			//Set the Projection matrix
			if (camStruct.type == CAMERA_TYPE::PERSPECTIVE)
			{
				(camProjection = DirectX::XMMatrixPerspectiveFovLH(camStruct.fovY, (float)w / h, camStruct.nearZ, camStruct.farZ));
			}
			else
			{
				(camProjection = DirectX::XMMatrixOrthographicLH(1, 1, camStruct.nearZ, camStruct.farZ));
			}
			camera->SetView(camView);
			camera->SetProjection(camProjection);
			camera->SetCameraPosition(camPosition);
		}
		else if (type == MessageType::EDITED_MESH_TOPOLOGY)
		{
			m_mModels->at(name)->RemakeTopology(verts);
		}
	}
}

size_t ComLib::nextSize()
{
	///+-+-+-+-+-+-+-+-+-+-+-+
	//nah
	return size_t();
	///+-+-+-+-+-+-+-+-+-+-+-+
}



//size_t ComLib::nextSize()
//{
//	size_t r = -1;
//	size_t sizeLeft;
//	size_t h = GetHead();
//	head - tail >= 0 ? sizeLeft = head - tail : sizeLeft = this->ringBufferSize - abs((int)(head - tail));
//
//	if (sizeLeft > sizeof(Header))
//	{
//		Header recv;
//		CopyMemory(&recv, this->ringBuffer.GetBuffer() + tail, sizeof(Header));
//		r = recv.msgLength;
//	}
//
//	return r;
//}

//bool ComLib::send(const void * msg, const size_t length)
//{
//	//*****************************************************************************
//
//	WaitForSingleObject(hnd_Mutex, INFINITE);
//	this->tail = this->GetTail();
//
//	bool tailIsBehind = (tail < head);
//
//	size_t sizeToEOB;
//	{
//		sizeToEOB = ringBufferSize - head;
//	}
//
//	size_t sizeToTail;
//	{
//		tailIsBehind ? sizeToTail = (sizeToEOB + tail - 1) % ringBufferSize: sizeToTail = tail - head - 1;
//	}
//
//	if (	(tailIsBehind && sizeToEOB >= length) ||
//			(!tailIsBehind && sizeToTail >= length) ||
//			(tail == head && sizeToEOB >= length))
//	{
//		char* cpMsg = (char*)msg;
//		char* pByte = (char*)this->pRingBuffer + head;
//		this->ringBuffer.Send(pByte, cpMsg, length);
//		head = (head + length) % this->ringBufferSize;
//		UpdateRBD(head);
//		RETURN_SAFE_TRUE;
//	}
//	else if (tailIsBehind && sizeToEOB < length && tail > 0)
//	{
//		//PAD
//		memset(ringBuffer.GetBuffer() + head, 0, ringBufferSize - head);
//		head = 0;
//		this->UpdateRBD(head);
//		RETURN_SAFE_FALSE;
//	}
//	else
//		RETURN_SAFE_FALSE;
//
//	//*****************************************************************************
//
//
//	if (this->type == ComLib::TYPE::PRODUCER)
//	{
//		//TODO REDO ALL
//		//TODO PAD
//
//		size_t sizeLeft;
//		if (tail == head) //TODO
//			sizeLeft = ringBufferSize;
//		else if (((head + 1) % ringBufferSize) == tail)
//			sizeLeft = 0;
//		else
//			tail > head ? sizeLeft = tail - head : sizeLeft = (ringBufferSize - head + tail);
//
//		//add padding so its multiple of 64. If message supplied
//		//is already padded, no padding will be added.
//		size_t lengthWithPad = length +(64 % length);
//
//		/****
//		The tail is "in front" of head in memory
//		****/
//		if (lengthWithPad > ringBufferSize - head && (tail > head)) //if there isnt enough space at end AND tail is in that section
//			RETURN_SAFE_FALSE; //return false. We are to wait for the tail to advance so we can safely pad the remaining memory
//
//		if (tail > head && tail - head < lengthWithPad) //the message fits in the remaining memory, but the tail is blocking us.
//			RETURN_SAFE_FALSE; //wait for tail to advance so we can send the message.
//
//		/****
//		The tail is "behind" head in memory
//		****/
//		if (tail < head && lengthWithPad > ringBufferSize - head) //not enough space. We pad now, advance head and return without sending.
//		{
//			memset(ringBuffer.GetBuffer() + head, 0, ringBufferSize - head);
//			head = 0;
//			this->UpdateRBD(head);
//			RETURN_SAFE_FALSE;
//		}
//
//		if (this->ringBufferSize - (tail - head) < lengthWithPad)
//			RETURN_SAFE_FALSE;
//
//		//should be safe to send now
//		char* cpMsg = (char*)msg;
//		char* pByte = (char*)this->pRingBuffer + head;
//		this->ringBuffer.Send(pByte, cpMsg, length);
//		head = (head + length) % this->ringBufferSize;
//		UpdateRBD(head);
//	}
//	else RETURN_SAFE_FALSE;
//
//	RETURN_SAFE_TRUE;
//
//}

//Overload for custom message
//bool ComLib::send(const Message& message, const size_t length)
//{
//	this->tail = this->GetTail();
//
//	if (this->type == ComLib::TYPE::PRODUCER)
//	{
//		//add padding so its multiple of 64. If message supplied
//		//is already padded, no padding will be added.
//		size_t lengthWithPad = length + (64 % length);
//
//		/****
//		The tail is "in front" of head in memory
//		****/
//		if ((length + (64 % length) > ringBufferSize - head) && (tail > head)) //if there isnt enough space at end AND tail is in that section
//			return false; //return false. We are to wait for the tail to advance so we can safely pad the remaining memory
//
//		if (tail > head && tail - head < length) //the message fits in the remaining memory, but the tail is blocking us.
//			return false; //wait for tail to advance so we can send the message.
//
//						  /****
//						  The tail is "behind" head in memory
//						  ****/
//		if (tail < head && length + (64 % length) > ringBufferSize - head) //not enough space. We pad now, advance head and return without sending.
//		{
//			memset(ringBuffer.GetBuffer() + head, 0, ringBufferSize - head);
//			head = 0;
//			this->UpdateRBD(head);
//			return false;
//		}
//
//		if (this->ringBufferSize - (tail - head) < lengthWithPad)
//			return false;
//
//		//should be safe to send now right?
//
//
//
//
//		//Check if there is enough space at end of buffer.
//		//If there isnt, pad end and send to start of buffer.
//		//size_t sizeLeft = ringBufferSize - head;
//
//		//if (lengthWithPad > sizeLeft && head >= tail)
//		//{
//		//	//pad end, move head and retry
//		//	memset(ringBuffer.GetBuffer(), -1, sizeLeft);
//		//	head = 0;
//		//}
//		//else
//		//{
//
//		//}
//
//		//size_t sizeUntilTail;
//		//CopyMemory(&sizeUntilTail, ringBufferData.GetBuffer() + sizeof(size_t), sizeof(size_t));
//		//sizeUntilTail -= head;
//		//
//		////If sizeUntilTail is negative here, it means the tail is "behind"
//		////the head, in a linear sense. 
//		////Recalculate sizeUntilTail
//		//{
//		//	if (sizeUntilTail < 0)
//		//		sizeUntilTail = ringBufferSize + sizeUntilTail;
//		//}
//
//
//		//if (sizeUntilTail < lengthWithPad)
//		//{
//
//		//}
//		//else
//		//{
//
//		//}
//		std::cout << "Sending Header at: " << (PVOID)this->ringBuffer.GetBuffer() << " + " << head << "(size of header: " << sizeof(ComLib::Header) << ")" <<std::endl;
//		char* pByte = (char*)this->pRingBuffer + head;
//		this->ringBuffer.Send(pByte, (PVOID)&message.hdr, sizeof(Header));
//		//TODO: REMOVE
//		Message recvMessage;
//		this->ringBuffer.Recieve((PVOID)&recvMessage.hdr, pByte, sizeof(Header));
//		//CopyMemory(&recvMessage.hdr, pByte, sizeof(Header));
//		int x = 1;
//		//////////////
//
//		pByte += sizeof(Header);
//		std::cout << "Sending msg at: " << (PVOID)this->ringBuffer.GetBuffer() << " + " << head+sizeof(Header) << "(size of message: " << message.hdr.msgLength << ")" << std::endl;
//		this->ringBuffer.Send(pByte, (PVOID)message.msg, sizeof(char) * message.hdr.msgLength);
//
//
//		head = (head + lengthWithPad) % this->ringBufferSize;
//		UpdateRBD(head);
//	}
//	else return false;
//
//	return true;
//
//}

//bool ComLib::recv(char * msg, size_t & length)
//{
//	//TODO: doublecheck
//	//TODO REDO ALL
//	WaitForSingleObject(hnd_Mutex, INFINITE);
//	if (this->type == ComLib::TYPE::CONSUMER)
//	{
//		this->head = GetHead();
//
//		bool headIsInFront = (head <= tail);
//		size_t sizeToHead;
//		headIsInFront ? sizeToHead = head - tail : sizeToHead = ringBufferSize - tail + head;
//
//		if (sizeToHead < length)
//			RETURN_SAFE_FALSE;
//		//Determine where to start reading
//		char* pByte = (char*)this->pRingBuffer + tail;
//
//		//size_t evalLength = strlen(pByte) + 1;
//		//if (evalLength <= 1)
//		//{
//		//	//Advance tail and return
//		//	if (tail == ringBufferSize)
//		//		tail = 0;
//		//	else tail += 1;
//		//	UpdateRBD(tail);
//		//	RETURN_SAFE;
//		//}
//		//std::cout << "\tEval len to: " << evalLength << std::endl;
//		//is there anything to recieve?
//		//if (head - tail < length)
//		//	RETURN_SAFE;
//
//		CopyMemory(msg, pByte, length);
//		if (msg[0] == '\0')
//		{
//			//Advance tail and return
//			if (tail == ringBufferSize)
//				tail = 0;
//			else tail += 1;
//			UpdateRBD(tail);
//			RETURN_SAFE_FALSE;
//		}
//		tail += length;
//		UpdateRBD(tail);
//
//		ReleaseMutex(hnd_Mutex);
//		return true;
//
//	}
//	else RETURN_SAFE_FALSE;
//
//	RETURN_SAFE_TRUE;
//}
