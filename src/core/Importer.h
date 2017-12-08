#ifndef CORE_IMPORTER_H_
#define CORE_IMPORTER_H_

#include "../support/TypeDefinitions.h"
#include "../support/helpers.h"
#include "../io/bcf/BCFReader.h"
#include "BlockEntry.h"
#include "../algorithm/permutation/RadixSortGT.h"
#include "../algorithm/compression/EncoderGenotypesRLE.h"
#include "../index/IndexEntry.h"
#include "ImportWriter.h"
#include "ImporterStats.h"

namespace Tachyon {

class Importer {
	typedef Importer self_type;
	typedef reader reader_type;
	typedef VCF::VCFHeader header_type;
	typedef ImportWriter writer_type;
	typedef IO::BasicBuffer buffer_type;
	typedef Algorithm::EncoderGenotypesRLE encoder_type;
	typedef Totempole::IndexEntry index_entry_type;
	typedef BCF::BCFReader bcf_reader_type;
	typedef BCF::BCFEntry bcf_entry_type;
	typedef Algorithm::RadixSortGT radix_sorter_type;
	typedef Core::StreamContainer stream_container;
	typedef Core::PermutationManager permutation_type;
	typedef Core::MetaHot meta_type;
	typedef Core::Support::HashContainer hash_container_type;
	typedef Core::Support::HashVectorContainer hash_vector_container_type;
	typedef Core::BlockEntry block_type;
	typedef Support::ImporterStats import_stats_type;

public:
	Importer(std::string inputFile, std::string outputPrefix, const U32 checkpoint_size, const double checkpoint_bases);
	~Importer();
	bool Build();

	inline void setPermute(const bool yes){ this->permute = yes; }

private:
	bool BuildBCF();  // import a BCF file
	bool parseBCFLine(bcf_entry_type& line); // Import a BCF line
	bool parseBCFBody(meta_type& meta, bcf_entry_type& line);

private:
	void resetHashes(void);

private:
	bool permute;
	U32 checkpoint_n_snps;      // number of variants until checkpointing
	double checkpoint_bases;

	import_stats_type import_uncompressed_stats;
	import_stats_type import_compressed_stats;

	// Read/write fields
	std::string inputFile;    // input file name
	std::string outputPrefix; // output file prefix
	reader_type reader_;      // reader
	writer_type writer_;      // writer

	index_entry_type index_entry;  // Header index
	radix_sorter_type permutator;
	header_type* header_;     // header
	encoder_type encoder;     // RLE packer

	block_type block;

	// Use during import
	// Never stored
	hash_container_type info_fields;
	hash_container_type format_fields;
	hash_container_type filter_fields;
	hash_vector_container_type info_patterns;
	hash_vector_container_type format_patterns;
	hash_vector_container_type filter_patterns;

	// Recycled buffer used to encode
	// or recode streams
	buffer_type recode_buffer;
};


} /* namespace Tachyon */

#endif /* CORE_IMPORTER_H_ */
