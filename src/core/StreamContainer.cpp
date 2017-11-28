#include "../support/TypeDefinitions.h"
#include "../support/helpers.h"
#include "../third_party/zlib/zconf.h"
#include "../third_party/zlib/zlib.h"
#include "../io/BasicBuffer.h"
#include "../algorithm/OpenHashTable.h"
#include "StreamContainer.h"

namespace Tachyon{
namespace Core{

bool StreamContainer::generateCRC(bool both){
	if(this->buffer_data.size() == 0)
		return false;

	// Checksum for main buffer
	U32 crc = crc32(0, NULL, 0);
	crc = crc32(crc, (Bytef*)this->buffer_data.data, this->buffer_data.pointer);
	this->header.crc = crc;

	// Checksum for strides
	if(both){
		if(this->buffer_strides.size() > 0){
			crc = crc32(0, NULL, 0);
			crc = crc32(crc, (Bytef*)this->buffer_strides.data, this->buffer_strides.pointer);
			this->header_stride.crc = crc;
		}
	}
	return true;
}

bool StreamContainer::checkCRC(int target){
	if(target == 0){
		if(this->buffer_data_uncompressed.size() == 0)
			return true;

		// Checksum for main buffer
		U32 crc = crc32(0, NULL, 0);
		crc = crc32(crc, (Bytef*)this->buffer_data_uncompressed.data, this->buffer_data_uncompressed.pointer);
		return(crc == this->header.crc);
	} else if(target == 1){
		if(this->buffer_strides_uncompressed.size() == 0)
			return true;

		// Checksum for main buffer
		U32 crc = crc32(0, NULL, 0);
		crc = crc32(crc, (Bytef*)this->buffer_strides_uncompressed.data, this->buffer_strides_uncompressed.pointer);
		return(crc == this->header.crc);
	} else if(target == 3){
		if(this->buffer_data.size() == 0)
			return true;

		// Checksum for main buffer
		U32 crc = crc32(0, NULL, 0);
		crc = crc32(crc, (Bytef*)this->buffer_data.data, this->buffer_data.pointer);
		return(crc == this->header.crc);
	} else if(target == 4){
		if(this->buffer_strides.size() == 0)
			return true;

		// Checksum for main buffer
		U32 crc = crc32(0, NULL, 0);
		crc = crc32(crc, (Bytef*)this->buffer_strides.data, this->buffer_strides.pointer);
		return(crc == this->header.crc);
	}
	return true;
}

bool StreamContainer::checkUniformity(void){
	if(this->n_entries == 0)
		return false;

	const S16& stride_size = this->header.stride;
	if(stride_size == -1)
		return false;

	U32 stride_update = stride_size;

	BYTE word_width = sizeof(char);
	switch(this->header.controller.type){
	case CORE_TYPE::TYPE_32B:   stride_update *= sizeof(S32);   word_width = sizeof(S32);   break;
	case CORE_TYPE::TYPE_FLOAT: stride_update *= sizeof(float); word_width = sizeof(float); break;
	case CORE_TYPE::TYPE_8B:    stride_update *= sizeof(char);  word_width = sizeof(char);  break;
	default: return false; break;
	}

	const U64 first_hash = XXH64(&this->buffer_data.data[0], stride_update, 2147483647);

	for(U32 i = 1; i < this->n_entries; ++i){
		if(XXH64(&this->buffer_data.data[i*stride_update], stride_update, 2147483647) != first_hash){
			//std::cerr << "not uniform" << std::endl;
			return(false);
		}
	}
	//std::cerr << "is uniform" << std::endl;

	this->n_entries = 1;
	this->n_additions = 1;
	// Data pointers are updated in case there is no reformatting
	// see StreamContainer::reformat()
	this->buffer_data.pointer = stride_size * word_width;
	this->header.uLength = stride_size * word_width;
	this->header.cLength = stride_size * word_width;
	this->header.controller.uniform = true;
	this->header.controller.mixedStride = false;
	this->header.controller.encoder = Core::ENCODE_NONE;

	return(true);
}

/*
 This function is called during import to shrink each
 word-type to fit min(x) and max(x) in the worst case.
 At this stage all integer values in the stream is of
 type S32. No other values can be shrunk
 */
void StreamContainer::reformat(buffer_type& buffer){
	// Recode integer types only
	if(!(this->header.controller.type == TYPE_32B && this->header.controller.signedness == 1))
		return;

	// At this point all integers are S32
	const S32* const dat = reinterpret_cast<const S32* const>(this->buffer_data.data);
	S32 min = dat[0];
	S32 max = dat[0];

	for(U32 j = 1; j < this->n_entries; ++j){
		if(dat[j] < min) min = dat[j];
		if(dat[j] > max) max = dat[j];
	}

	BYTE byte_width = 0;
	if(min < 0) byte_width = ceil((ceil(log2(abs(min) + 1)) + 1) / 8);  // One bit is used for sign
	else byte_width = ceil(ceil(log2(max + 1)) / 8);

	if(byte_width >= 3 && byte_width <= 4) byte_width = 4;
	else if(byte_width > 4) byte_width = 8;
	if(byte_width == 0) byte_width = 1;

	// Phase 2
	// Here we re-encode values using the smallest possible
	// word-size
	if(this->header.controller.uniform){
		this->buffer_data.reset();

		// Non-negative
		// Controller for the unform case has already been setup
		if(min >= 0){
			this->header.controller.signedness = 0;
			switch(byte_width){
			case 1: this->buffer_data += (BYTE)min; this->header.controller.type = TYPE_8B;  break;
			case 2: this->buffer_data += (U16)min;  this->header.controller.type = TYPE_16B; break;
			case 4: this->buffer_data += (U32)min;  this->header.controller.type = TYPE_32B; break;
			case 8: this->buffer_data += (U64)min;  this->header.controller.type = TYPE_64B; break;
			default: std::cerr << "illegal: " << std::endl; exit(1);
			}
		} else {
			this->header.controller.signedness = 1;
			switch(byte_width){
			case 1: this->buffer_data += (SBYTE)min; this->header.controller.type = TYPE_8B;  break;
			case 2: this->buffer_data += (S16)min;   this->header.controller.type = TYPE_16B; break;
			case 4: this->buffer_data += (S32)min;   this->header.controller.type = TYPE_32B; break;
			default: std::cerr << "illegal" << std::endl; exit(1);
			}
		}
		// done
		this->header.uLength = this->buffer_data.pointer;
		this->header.cLength = this->buffer_data.pointer;

		return;
	}
	// Not unfirom
	else {
		buffer.reset();
		buffer.resize(this->buffer_data.pointer);

		// Is non-negative
		if(min >= 0){
			this->header.controller.signedness = 0;

			if(byte_width == 1){
				this->header.controller.type = TYPE_8B;

				for(U32 j = 0; j < this->n_entries; ++j)
					buffer += (BYTE)dat[j];
			} else if(byte_width == 2){
				this->header.controller.type = TYPE_16B;

				for(U32 j = 0; j < this->n_entries; ++j)
					buffer += (U16)dat[j];
			} else if(byte_width == 4){
				this->header.controller.type = TYPE_32B;

				for(U32 j = 0; j < this->n_entries; ++j)
					buffer += (U32)dat[j];
			} else if(byte_width == 8){
				this->header.controller.type = TYPE_64B;

				for(U32 j = 0; j < this->n_entries; ++j)
					buffer += (U64)dat[j];
			} else {
				std::cerr << "illegal" << std::endl;
				exit(1);
			}
		}
		// Is negative
		else {
			this->header.controller.signedness = 1;

			if(byte_width == 1){
				this->header.controller.type = TYPE_8B;

				for(U32 j = 0; j < this->n_entries; ++j)
					buffer += (SBYTE)dat[j];
			} else if(byte_width == 2){
				this->header.controller.type = TYPE_16B;

				for(U32 j = 0; j < this->n_entries; ++j)
					buffer += (S16)dat[j];
			} else if(byte_width == 4){
				this->header.controller.type = TYPE_32B;

				for(U32 j = 0; j < this->n_entries; ++j)
					buffer += (S32)dat[j];
			} else {
				std::cerr << "illegal" << std::endl;
				exit(1);
			}
		}
		//std::cerr << "recode shrink: " << this->buffer_data.pointer << '\t' << buffer.pointer << std::endl;
		memcpy(this->buffer_data.data, buffer.data, buffer.pointer);
		this->buffer_data.pointer = buffer.pointer;
		this->header.uLength = this->buffer_data.pointer;
	}
}

void StreamContainer::reformatStride(buffer_type& buffer){
	// Recode integer types
	if(!(this->header_stride.controller.type == TYPE_32B && this->header_stride.controller.signedness == 0)){
		return;
	}

	// At this point all integers are S32
	const U32* const dat = reinterpret_cast<const U32* const>(this->buffer_strides.data);
	U32 max = dat[0];

	for(U32 j = 1; j < this->n_entries; ++j){
		if(dat[j] > max) max = dat[j];
	}

	BYTE byte_width = ceil(ceil(log2(max + 1))/8);

	if(byte_width >= 3 && byte_width <= 4) byte_width = 4;
	else if(byte_width > 4) byte_width = 8;
	if(byte_width == 0) byte_width = 1;

	// This cannot ever be uniform
	buffer.reset();
	buffer.resize(this->buffer_strides.pointer);


	if(byte_width == 1){
		this->header_stride.controller.type = TYPE_8B;

		for(U32 j = 0; j < this->n_entries; ++j)
			buffer += (BYTE)dat[j];
	} else if(byte_width == 2){
		this->header_stride.controller.type = TYPE_16B;

		for(U32 j = 0; j < this->n_entries; ++j)
			buffer += (U16)dat[j];
	} else if(byte_width == 4){
		this->header_stride.controller.type = TYPE_32B;

		for(U32 j = 0; j < this->n_entries; ++j)
			buffer += (U32)dat[j];
	} else if(byte_width == 8){
		this->header_stride.controller.type = TYPE_64B;

		for(U32 j = 0; j < this->n_entries; ++j)
			buffer += (U64)dat[j];
	} else {
		std::cerr << "illegal" << std::endl;
		exit(1);
	}
	//std::cerr << "recode shrink strides: " << this->buffer_strides.pointer << '\t' << buffer.pointer << std::endl;
	memcpy(this->buffer_strides.data, buffer.data, buffer.pointer);
	this->buffer_strides.pointer = buffer.pointer;
}

/////////////////////
// Loading a container
/////////////////////
// Sketch of algorithm
// 1) Load header
// 2) Read compressed data into temp buffer
// 3) Decompress into local buffer
// 4) Finished if stride is not equal to -1
// 5) Load stride header
// 6) Read stride data into temp buffer
// 7) Decompress into local buffer
bool StreamContainer::read(std::ifstream& stream, buffer_type& temp_buffer){
	stream >> this->header;
	// Todo: currently no encoding-specific fields
	stream.read(temp_buffer.data, this->header.cLength);
	if(!stream.good()){
		std::cerr << Helpers::timestamp("ERROR","CONTAINER") << "Stream is not good!" << std::endl;
		return false;
	}
	// Uncompress into local buffer

	if(this->header.controller.mixedStride){
		stream >> this->header_stride;
		stream.read(temp_buffer.data, this->header_stride.cLength);
		if(!stream.good()){
			std::cerr << Helpers::timestamp("ERROR","CONTAINER") << "Stream is not good!" << std::endl;
			return false;
		}
		// Uncompress into local buffer
	} // end stride extra

	// check crc checksum

	return true;
}

}
}
