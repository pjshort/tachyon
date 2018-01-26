#ifndef INDEX_INDEXBLOCKENTRYOFFSETS_H_
#define INDEX_INDEXBLOCKENTRYOFFSETS_H_

#include "../ContainerHeaderController.h"
#include "../ContainerHeader.h"

namespace tachyon{
namespace containers{
namespace core{

struct DataBlockOffsetsHeader{
	typedef DataBlockOffsetsHeader self_type;

	DataBlockOffsetsHeader() : key(0), offset(0){}

	void clear(void){
		this->key = 0;
		this->offset = 0;
	}

	friend std::ofstream& operator<<(std::ofstream& stream, const self_type& entry){
		stream.write(reinterpret_cast<const char*>(&entry.key),    sizeof(U32));
		stream.write(reinterpret_cast<const char*>(&entry.offset), sizeof(U32));
		return stream;
	}

	friend std::ifstream& operator>>(std::ifstream& stream, self_type& entry){
		stream.read(reinterpret_cast<char*>(&entry.key),    sizeof(U32));
		stream.read(reinterpret_cast<char*>(&entry.offset), sizeof(U32));
		return stream;
	}

	U32 key;
	U32 offset;
};

struct DataBlockOffsets{
	typedef DataBlockOffsets self_type;
	typedef ContainerHeader header_type;
	typedef ContainerHeaderStride header_stride_type;

public:
	DataBlockOffsets(void) : key(0){}
	DataBlockOffsets(const U32& key, const header_type& h) : key(key), header(h){}
	DataBlockOffsets(const U32& key, const header_type& h, const header_stride_type& s) : key(key), header(h), header_stride(s){}
	~DataBlockOffsets(void){}

	inline bool update(const U32& key){
		this->key = key;
		return true;
	}

	inline bool update(const U32& key, const header_type& h){
		this->key = key;
		this->header = h;
		return true;
	}

	inline bool update(const U32& key, const header_type& h, const header_stride_type& s){
		this->key = key;
		this->header = h;
		this->header_stride = s;
		return true;
	}

	inline bool update(const header_type& h){
		this->header = h;
		return true;
	}

	inline bool update(const header_type& h, const header_stride_type& s){
		this->header = h;
		this->header_stride = s;
		return true;
	}

	inline void clear(void){
		this->key = 0;
		this->header.reset();
		this->header_stride.reset();
	}

	friend std::ofstream& operator<<(std::ofstream& stream, const self_type& entry){
		stream.write(reinterpret_cast<const char*>(&entry.key), sizeof(U32));
		stream << entry.header;
		if(entry.header.controller.mixedStride)
			stream << entry.header_stride;

		return(stream);
	}

	friend std::ifstream& operator>>(std::ifstream& stream, self_type& entry){
		stream.read(reinterpret_cast<char*>(&entry.key), sizeof(U32));
		stream >> entry.header;
		if(entry.header.controller.mixedStride)
			stream >> entry.header_stride;

		return(stream);
	}

public:
	U32 key;
	header_type header;
	header_stride_type header_stride;
};

}
}
}

#endif /* INDEX_INDEXBLOCKENTRYOFFSETS_H_ */