#include <cassert>

#include "variantblock_footer.h"

namespace tachyon{
namespace containers{

VariantBlockFooter::VariantBlockFooter():
	l_info_bitvector(0),
	l_format_bitvector(0),
	l_filter_bitvector(0),
	n_info_streams(0),
	n_format_streams(0),
	n_filter_streams(0),
	n_info_patterns(0),
	n_format_patterns(0),
	n_filter_patterns(0),
	info_offsets(nullptr),
	format_offsets(nullptr),
	filter_offsets(nullptr),
	info_bit_vectors(nullptr),
	format_bit_vectors(nullptr),
	filter_bit_vectors(nullptr)
{}

VariantBlockFooter::~VariantBlockFooter(){
	delete [] this->info_offsets;
	delete [] this->format_offsets;
	delete [] this->filter_offsets;
	delete [] this->info_bit_vectors;
	delete [] this->format_bit_vectors;
	delete [] this->filter_bit_vectors;
}

void VariantBlockFooter::reset(void){
	// Headers of the various containers
	this->offset_ppa.reset();
	this->offset_meta_contig.reset();
	this->offset_meta_position.reset();
	this->offset_meta_refalt.reset();
	this->offset_meta_controllers.reset();
	this->offset_meta_quality.reset();
	this->offset_meta_names.reset();
	this->offset_meta_alleles.reset();
	this->offset_meta_info_id.reset();
	this->offset_meta_format_id.reset();
	this->offset_meta_filter_id.reset();
	this->offset_gt_8b.reset();
	this->offset_gt_16b.reset();
	this->offset_gt_32b.reset();
	this->offset_gt_64b.reset();
	this->offset_gt_simple8.reset();
	this->offset_gt_simple16.reset();
	this->offset_gt_simple32.reset();
	this->offset_gt_simple64.reset();
	this->offset_gt_helper.reset();

	delete [] this->info_offsets;
	delete [] this->format_offsets;
	delete [] this->filter_offsets;
	this->info_offsets   = nullptr;
	this->format_offsets = nullptr;
	this->filter_offsets = nullptr;

	// Bit vectors
	delete [] this->info_bit_vectors;
	delete [] this->format_bit_vectors;
	delete [] this->filter_bit_vectors;
	this->info_bit_vectors   = nullptr;
	this->format_bit_vectors = nullptr;
	this->filter_bit_vectors = nullptr;

	this->n_info_streams    = 0;
	this->n_format_streams  = 0;
	this->n_filter_streams  = 0;
	this->n_info_patterns   = 0;
	this->n_format_patterns = 0;
	this->n_filter_patterns = 0;
}

bool VariantBlockFooter::constructBitVector(const INDEX_BLOCK_TARGET& target, hash_container_type& values, hash_vector_container_type& patterns){
	if(values.size() == 0)
		return false;

	// Determine target
	switch(target){
	case(INDEX_BLOCK_TARGET::INDEX_INFO)   :
		this->n_info_patterns = patterns.size();
		return(this->__constructBitVector(this->info_bit_vectors, this->info_offsets,  values, patterns));
		break;
	case(INDEX_BLOCK_TARGET::INDEX_FORMAT) :
		this->n_format_patterns = patterns.size();
		return(this->__constructBitVector(this->format_bit_vectors, this->format_offsets, values, patterns));
		break;
	case(INDEX_BLOCK_TARGET::INDEX_FILTER) :
		this->n_filter_patterns = patterns.size();
		return(this->__constructBitVector(this->filter_bit_vectors, this->filter_offsets, values, patterns));
		break;
	default: std::cerr << "unknown target type" << std::endl; exit(1);
	}

	return false;
}

bool VariantBlockFooter::__constructBitVector(bit_vector*& target,
                                             header_type*  offset,
                                     hash_container_type&  values,
                              hash_vector_container_type& patterns)
{
	if(values.size() == 0) return false;

	// Determine the required width in bytes of the bit-vector
	BYTE bitvector_width = ceil((float)values.size()/8);

	// Allocate new bit-vectors
	delete [] target;
	target = new bit_vector[patterns.size()];

	// Allocate memory for these bit-vectors
	for(U32 i = 0; i < patterns.size(); ++i)
		target[i].allocate(patterns[i].size(), bitvector_width);

	// Cycle over pattern size
	for(U32 i = 0; i < patterns.size(); ++i){
		for(U32 j = 0; j < patterns[i].size(); ++j){
			// Set arbitrary local key: this value is update by reference in `getRaw()`
			U32 local_key = 0;

			// Map from absolute key to local key
			if(!values.getRaw(patterns[i][j], local_key)){
				std::cerr << "impossible to get " << patterns[i][j] << std::endl;
				exit(1);
			}

			// Set bit at local key position
			target[i].bit_bytes[local_key/8] |= 1 << (local_key % 8);

			// Store local key in key-chain
			target[i].local_keys[j] = local_key;

			// Store absolute key
			offset[local_key].data_header.global_key = patterns[i][j];
		}
	}
	return true;
}

std::ostream& operator<<(std::ostream& stream, const VariantBlockFooter& entry){
	stream.write(reinterpret_cast<const char*>(&entry.n_info_streams),    sizeof(U16));
	stream.write(reinterpret_cast<const char*>(&entry.n_format_streams),  sizeof(U16));
	stream.write(reinterpret_cast<const char*>(&entry.n_filter_streams),  sizeof(U16));
	stream.write(reinterpret_cast<const char*>(&entry.n_info_patterns),   sizeof(U16));
	stream.write(reinterpret_cast<const char*>(&entry.n_format_patterns), sizeof(U16));
	stream.write(reinterpret_cast<const char*>(&entry.n_filter_patterns), sizeof(U16));

	stream << entry.offset_ppa;
	stream << entry.offset_meta_contig;
	stream << entry.offset_meta_position;
	stream << entry.offset_meta_refalt;
	stream << entry.offset_meta_controllers;
	stream << entry.offset_meta_quality;
	stream << entry.offset_meta_names;
	stream << entry.offset_meta_alleles;
	stream << entry.offset_meta_info_id;
	stream << entry.offset_meta_format_id;
	stream << entry.offset_meta_filter_id;
	stream << entry.offset_gt_8b;
	stream << entry.offset_gt_16b;
	stream << entry.offset_gt_32b;
	stream << entry.offset_gt_64b;
	stream << entry.offset_gt_simple8;
	stream << entry.offset_gt_simple16;
	stream << entry.offset_gt_simple32;
	stream << entry.offset_gt_simple64;
	stream << entry.offset_gt_helper;

	for(U32 i = 0; i < entry.n_info_streams; ++i)   stream << entry.info_offsets[i];
	for(U32 i = 0; i < entry.n_format_streams; ++i) stream << entry.format_offsets[i];
	for(U32 i = 0; i < entry.n_filter_streams; ++i) stream << entry.filter_offsets[i];

	// write
	if(entry.n_info_patterns){
		const BYTE info_bitvector_width = ceil((float)entry.n_info_streams/8);
		for(U32 i = 0; i < entry.n_info_patterns; ++i){
			stream << entry.info_bit_vectors[i];
			stream.write((const char*)entry.info_bit_vectors[i].bit_bytes, info_bitvector_width);
		}
	}

	if(entry.n_format_patterns){
		const BYTE format_bitvector_width = ceil((float)entry.n_format_streams/8);
		for(U32 i = 0; i < entry.n_format_patterns; ++i){
			stream << entry.format_bit_vectors[i];
			stream.write((const char*)entry.format_bit_vectors[i].bit_bytes, format_bitvector_width);
		}
	}

	if(entry.n_filter_patterns){
		const BYTE filter_bitvector_width = ceil((float)entry.n_filter_streams/8);
		for(U32 i = 0; i < entry.n_filter_patterns; ++i){
			stream << entry.filter_bit_vectors[i];
			stream.write((const char*)entry.filter_bit_vectors[i].bit_bytes, filter_bitvector_width);
		}
	}

	return(stream);
}

std::ifstream& operator>>(std::ifstream& stream, VariantBlockFooter& entry){
	stream.read(reinterpret_cast<char*>(&entry.n_info_streams),    sizeof(U16));
	stream.read(reinterpret_cast<char*>(&entry.n_format_streams),  sizeof(U16));
	stream.read(reinterpret_cast<char*>(&entry.n_filter_streams),  sizeof(U16));
	stream.read(reinterpret_cast<char*>(&entry.n_info_patterns),   sizeof(U16));
	stream.read(reinterpret_cast<char*>(&entry.n_format_patterns), sizeof(U16));
	stream.read(reinterpret_cast<char*>(&entry.n_filter_patterns), sizeof(U16));

	entry.l_info_bitvector   = ceil((float)entry.n_info_streams/8);
	entry.l_format_bitvector = ceil((float)entry.n_format_streams/8);
	entry.l_filter_bitvector = ceil((float)entry.n_filter_streams/8);

	stream >> entry.offset_ppa;
	stream >> entry.offset_meta_contig;
	stream >> entry.offset_meta_position;
	stream >> entry.offset_meta_refalt;
	stream >> entry.offset_meta_controllers;
	stream >> entry.offset_meta_quality;
	stream >> entry.offset_meta_names;
	stream >> entry.offset_meta_alleles;
	stream >> entry.offset_meta_info_id;
	stream >> entry.offset_meta_format_id;
	stream >> entry.offset_meta_filter_id;
	stream >> entry.offset_gt_8b;
	stream >> entry.offset_gt_16b;
	stream >> entry.offset_gt_32b;
	stream >> entry.offset_gt_64b;
	stream >> entry.offset_gt_simple8;
	stream >> entry.offset_gt_simple16;
	stream >> entry.offset_gt_simple32;
	stream >> entry.offset_gt_simple64;
	stream >> entry.offset_gt_helper;

	entry.info_offsets   = new DataContainerHeader[entry.n_info_streams];
	entry.format_offsets = new DataContainerHeader[entry.n_format_streams];
	entry.filter_offsets = new DataContainerHeader[entry.n_filter_streams];
	for(U32 i = 0; i < entry.n_info_streams; ++i)   stream >> entry.info_offsets[i];
	for(U32 i = 0; i < entry.n_format_streams; ++i) stream >> entry.format_offsets[i];
	for(U32 i = 0; i < entry.n_filter_streams; ++i) stream >> entry.filter_offsets[i];

	if(entry.n_info_patterns){
		BYTE info_bitvector_width = ceil((float)entry.n_info_streams/8);
		entry.info_bit_vectors = new DataBlockBitvector[entry.n_info_patterns];
		for(U32 i = 0; i < entry.n_info_patterns; ++i){
			stream >> entry.info_bit_vectors[i];
			entry.info_bit_vectors[i].allocate(info_bitvector_width);
			stream.read((char*)entry.info_bit_vectors[i].bit_bytes, info_bitvector_width);
		}
	}

	if(entry.n_format_patterns){
		BYTE format_bitvector_width = ceil((float)entry.n_format_streams/8);
		entry.format_bit_vectors = new DataBlockBitvector[entry.n_format_patterns];
		for(U32 i = 0; i < entry.n_format_patterns; ++i){
			stream >> entry.format_bit_vectors[i];
			entry.format_bit_vectors[i].allocate(format_bitvector_width);
			stream.read((char*)entry.format_bit_vectors[i].bit_bytes, format_bitvector_width);
		}
	}

	if(entry.n_filter_patterns){
		BYTE filter_bitvector_width = ceil((float)entry.n_filter_streams/8);
		entry.filter_bit_vectors = new DataBlockBitvector[entry.n_filter_patterns];
		for(U32 i = 0; i < entry.n_filter_patterns; ++i){
			stream >> entry.filter_bit_vectors[i];
			entry.filter_bit_vectors[i].allocate(filter_bitvector_width);
			stream.read((char*)entry.filter_bit_vectors[i].bit_bytes, filter_bitvector_width);
		}
	}

	return(stream);
}

io::BasicBuffer& operator<<(io::BasicBuffer& buffer, const VariantBlockFooter& entry){
	buffer += (U16)entry.n_info_streams;
	buffer += (U16)entry.n_format_streams;
	buffer += (U16)entry.n_filter_streams;
	buffer += (U16)entry.n_info_patterns;
	buffer += (U16)entry.n_format_patterns;
	buffer += (U16)entry.n_filter_patterns;

	buffer << entry.offset_ppa;
	buffer << entry.offset_meta_contig;
	buffer << entry.offset_meta_position;
	buffer << entry.offset_meta_refalt;
	buffer << entry.offset_meta_controllers;
	buffer << entry.offset_meta_quality;
	buffer << entry.offset_meta_names;
	buffer << entry.offset_meta_alleles;
	buffer << entry.offset_meta_info_id;
	buffer << entry.offset_meta_format_id;
	buffer << entry.offset_meta_filter_id;
	buffer << entry.offset_gt_8b;
	buffer << entry.offset_gt_16b;
	buffer << entry.offset_gt_32b;
	buffer << entry.offset_gt_64b;
	buffer << entry.offset_gt_simple8;
	buffer << entry.offset_gt_simple16;
	buffer << entry.offset_gt_simple32;
	buffer << entry.offset_gt_simple64;
	buffer << entry.offset_gt_helper;

	for(U32 i = 0; i < entry.n_info_streams; ++i)   buffer << entry.info_offsets[i];
	for(U32 i = 0; i < entry.n_format_streams; ++i) buffer << entry.format_offsets[i];
	for(U32 i = 0; i < entry.n_filter_streams; ++i) buffer << entry.filter_offsets[i];

	if(entry.n_info_patterns > 0){
		const BYTE info_bitvector_width = ceil((float)entry.n_info_streams/8);
		for(U32 i = 0; i < entry.n_info_patterns; ++i){
			buffer << entry.info_bit_vectors[i];
			buffer.Add((const char* const)&entry.info_bit_vectors[i].bit_bytes[0], info_bitvector_width);
		}
	}

	if(entry.n_format_patterns > 0){
		const BYTE format_bitvector_width = ceil((float)entry.n_format_streams/8);
		for(U32 i = 0; i < entry.n_format_patterns; ++i){
			buffer << entry.format_bit_vectors[i];
			buffer.Add((const char* const)&entry.format_bit_vectors[i].bit_bytes[0], format_bitvector_width);
		}
	}

	if(entry.n_filter_patterns > 0){
		const BYTE filter_bitvector_width = ceil((float)entry.n_filter_streams/8);
		for(U32 i = 0; i < entry.n_filter_patterns; ++i){
			buffer << entry.filter_bit_vectors[i];
			buffer.Add((const char* const)&entry.filter_bit_vectors[i].bit_bytes[0], filter_bitvector_width);
		}
	}

	return(buffer);
}


io::BasicBuffer& operator>>(io::BasicBuffer& buffer, VariantBlockFooter& entry){
	buffer >> entry.n_info_streams;
	buffer >> entry.n_format_streams;
	buffer >> entry.n_filter_streams;
	buffer >> entry.n_info_patterns;
	buffer >> entry.n_format_patterns;
	buffer >> entry.n_filter_patterns;

	entry.l_info_bitvector   = ceil((float)entry.n_info_streams/8);
	entry.l_format_bitvector = ceil((float)entry.n_format_streams/8);
	entry.l_filter_bitvector = ceil((float)entry.n_filter_streams/8);

	buffer >> entry.offset_ppa;
	buffer >> entry.offset_meta_contig;
	buffer >> entry.offset_meta_position;
	buffer >> entry.offset_meta_refalt;
	buffer >> entry.offset_meta_controllers;
	buffer >> entry.offset_meta_quality;
	buffer >> entry.offset_meta_names;
	buffer >> entry.offset_meta_alleles;
	buffer >> entry.offset_meta_info_id;
	buffer >> entry.offset_meta_format_id;
	buffer >> entry.offset_meta_filter_id;
	buffer >> entry.offset_gt_8b;
	buffer >> entry.offset_gt_16b;
	buffer >> entry.offset_gt_32b;
	buffer >> entry.offset_gt_64b;
	buffer >> entry.offset_gt_simple8;
	buffer >> entry.offset_gt_simple16;
	buffer >> entry.offset_gt_simple32;
	buffer >> entry.offset_gt_simple64;
	buffer >> entry.offset_gt_helper;

	entry.info_offsets   = new DataContainerHeader[entry.n_info_streams];
	entry.format_offsets = new DataContainerHeader[entry.n_format_streams];
	entry.filter_offsets = new DataContainerHeader[entry.n_filter_streams];
	for(U32 i = 0; i < entry.n_info_streams; ++i)   buffer >> entry.info_offsets[i];
	for(U32 i = 0; i < entry.n_format_streams; ++i) buffer >> entry.format_offsets[i];
	for(U32 i = 0; i < entry.n_filter_streams; ++i) buffer >> entry.filter_offsets[i];

	if(entry.n_info_patterns){
		BYTE info_bitvector_width = ceil((float)entry.n_info_streams/8);
		entry.info_bit_vectors = new DataBlockBitvector[entry.n_info_patterns];
		for(U32 i = 0; i < entry.n_info_patterns; ++i){
			buffer >> entry.info_bit_vectors[i];
			entry.info_bit_vectors[i].allocate(info_bitvector_width);
			buffer.read((char*)entry.info_bit_vectors[i].bit_bytes, info_bitvector_width);
		}
	}

	if(entry.n_format_patterns){
		BYTE format_bitvector_width = ceil((float)entry.n_format_streams/8);
		entry.format_bit_vectors = new DataBlockBitvector[entry.n_format_patterns];
		for(U32 i = 0; i < entry.n_format_patterns; ++i){
			buffer >> entry.format_bit_vectors[i];
			entry.format_bit_vectors[i].allocate(format_bitvector_width);
			buffer.read((char*)entry.format_bit_vectors[i].bit_bytes, format_bitvector_width);
		}
	}

	if(entry.n_filter_patterns){
		BYTE filter_bitvector_width = ceil((float)entry.n_filter_streams/8);
		entry.filter_bit_vectors = new DataBlockBitvector[entry.n_filter_patterns];
		for(U32 i = 0; i < entry.n_filter_patterns; ++i){
			buffer >> entry.filter_bit_vectors[i];
			entry.filter_bit_vectors[i].allocate(filter_bitvector_width);
			buffer.read((char*)entry.filter_bit_vectors[i].bit_bytes, filter_bitvector_width);
		}
	}

	return(buffer);
}

}
}
