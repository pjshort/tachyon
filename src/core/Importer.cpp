#include <fstream>

#include "Importer.h"
#include "../algorithm/compression/CompressionContainer.h"
#include "base/MetaCold.h"
#include "../algorithm/DigitalDigestController.h"

namespace Tachyon {

#define IMPORT_ASSERT 1

Importer::Importer(std::string inputFile, std::string outputPrefix, const U32 checkpoint_n_snps, const double checkpoint_bases) :
	permute(true),
	checkpoint_n_snps(checkpoint_n_snps),
	checkpoint_bases(checkpoint_bases),
	inputFile(inputFile),
	outputPrefix(outputPrefix),
	reader_(inputFile),
	writer_(),
	header_(nullptr)
{
}

Importer::~Importer(){ this->recode_buffer.deleteAll(); }

void Importer::resetHashes(void){
	this->info_fields.clear();
	this->info_patterns.clear();
	this->format_fields.clear();
	this->format_patterns.clear();
	this->filter_fields.clear();
	this->filter_patterns.clear();
}

bool Importer::Build(){
	std::ifstream temp(this->inputFile, std::ios::binary | std::ios::in);
	if(!temp.good()){
		std::cerr << Helpers::timestamp("ERROR", "IMPORT")  << "Failed to open file (" << this->inputFile << ")..." << std::endl;
		return false;
	}
	char tempData[2];
	temp.read(&tempData[0], 2);
	temp.close();

	if((BYTE)tempData[0] == IO::Constants::GZIP_ID1 && (BYTE)tempData[1] == IO::Constants::GZIP_ID2){
		if(!this->BuildBCF()){
			std::cerr << Helpers::timestamp("ERROR", "IMPORT") << "Failed build!" << std::endl;
			return false;
		}
	} else {
		std::cerr << Helpers::timestamp("ERROR", "IMPORT") << "Unknown file format!" << std::endl;
		return false;
	}
	return true;
}

bool Importer::BuildBCF(void){
	bcf_reader_type reader;
	if(!reader.open(this->inputFile)){
		std::cerr << Helpers::timestamp("ERROR", "BCF")  << "Failed to open BCF file..." << std::endl;
		return false;
	}

	this->header_ = &reader.header;
	if(this->header_->samples == 0){
		std::cerr << Helpers::timestamp("ERROR", "BCF") << "No samples detected in header..." << std::endl;
		return false;
	}

	if(this->header_->samples == 1){
		std::cerr << Helpers::timestamp("ERROR", "IMPORT") << "Cannot run " << Tachyon::Constants::PROGRAM_NAME << " with a single sample..." << std::endl;
		return false;
	}

	// Spawn RLE controller and update PPA controller
	this->encoder.setSamples(this->header_->samples);
	this->block.ppa_manager.setSamples(this->header_->samples);
	this->permutator.manager = &this->block.ppa_manager;
	this->permutator.setSamples(this->header_->samples);

	if(!this->writer_.Open(this->outputPrefix)){
		std::cerr << Helpers::timestamp("ERROR", "WRITER") << "Failed to open writer..." << std::endl;
		return false;
	}

	if(!this->writer_.WriteHeader()){
		std::cerr << Helpers::timestamp("ERROR", "WRITER") << "Failed to write header..." << std::endl;
		return false;
	}
	this->writer_.streamTomahawk << *this->header_;

	// Resize containers
	const U32 resize_to = this->checkpoint_n_snps * sizeof(U32) * this->header_->samples * 100;
	this->block.resize(resize_to);

	Compression::ZSTDCodec zstd;

	// Digest controller
	Algorithm::DigitalDigestController* digests = new Algorithm::DigitalDigestController[this->header_->map.size()];
	for(U32 i = 0; i < this->header_->map.size(); ++i){
		if(!digests[i].initialize()){
			std::cerr << "failed to init sha512" << std::endl;
			return false;
		}
	}

	// Start import
	U32 previousFirst    = 0;
	U32 previousLast     = 0;
	S32 previousContigID = -1;
	U64 n_variants_read  = 0;

	// Begin import
	// Get BCF entries
	bcf_entry_type t;
	while(true){
		if(!reader.getVariants(this->checkpoint_n_snps, this->checkpoint_bases)){
			std::cerr << "breaking" << std::endl;
			break;
		}

		// Debug assertion
#if IMPORT_ASSERT == 1
		if(reader.first().body->CHROM == previousContigID){
			if(!(reader.first().body->POS >= previousFirst && reader.first().body->POS >= previousLast)){
				std::cerr << Helpers::timestamp("ERROR","IMPORT") << reader.first().body->POS << '/' << previousFirst << '/' << previousLast << std::endl;
				std::cerr << reader[reader.n_entries].body->POS << std::endl;
				exit(1);
			}
		}
#endif
		std::cerr << Helpers::timestamp("DEBUG") << "n_variants: " << reader.size() << '\t' << reader.first().body->POS+1 << "->" << reader.last().body->POS+1 << std::endl;
		this->block.index_entry.contigID    = reader.first().body->CHROM;
		this->block.index_entry.minPosition = reader.first().body->POS;
		this->block.index_entry.maxPosition = reader.last().body->POS;
		this->block.index_entry.controller.hasGTPermuted = this->permute;

		// Permute or not?
		if(this->block.index_entry.controller.hasGTPermuted){
			if(!this->permutator.build(reader)){
				std::cerr << Helpers::timestamp("ERROR","PERMUTE") << "Failed to complete..." << std::endl;
				return false;
			}
		}

		// Perform parsing of BCF entries in memory
		for(U32 i = 0; i < reader.size(); ++i){
			if(!this->parseBCFLine(reader[i])){
				std::cerr << Helpers::timestamp("ERROR","IMPORT") << "Failed to parse BCF body..." << std::endl;
				return false;
			}
		}

		// Stats
		n_variants_read += reader.size();

		assert(this->block.gt_support_data_container.n_entries == reader.size());
		assert(this->block.meta_info_map_ids.n_entries         == reader.size());
		assert(this->block.meta_filter_map_ids.n_entries       == reader.size());
		assert(this->block.meta_format_map_ids.n_entries       == reader.size());

		// Update head meta
		this->block.index_entry.controller.hasGT     = true;
		this->block.index_entry.controller.isDiploid = true;
		this->block.index_entry.n_info_streams       = this->info_fields.size();
		this->block.index_entry.n_filter_streams     = this->filter_fields.size();
		this->block.index_entry.n_format_streams     = this->format_fields.size();
		this->block.index_entry.n_variants           = reader.size();
		this->block.allocateOffsets(this->info_fields.size(), this->format_fields.size(), this->filter_fields.size());
		this->block.index_entry.constructBitVector(Index::IndexBlockEntry::INDEX_INFO,   this->info_fields,   this->info_patterns);
		this->block.index_entry.constructBitVector(Index::IndexBlockEntry::INDEX_FILTER, this->filter_fields, this->filter_patterns);
		this->block.index_entry.constructBitVector(Index::IndexBlockEntry::INDEX_FORMAT, this->format_fields, this->format_patterns);
		this->block.updateBaseContainers();
		this->block.updateContainerSet(Index::IndexBlockEntry::INDEX_INFO);
		this->block.updateContainerSet(Index::IndexBlockEntry::INDEX_FORMAT);

		zstd.setCompressionLevel(6);
		if(this->block.index_entry.controller.hasGTPermuted) zstd.encode(this->block.ppa_manager);

		zstd.setCompressionLevel(20);
		if(this->block.meta_hot_container.n_entries)        zstd.encode(this->block.meta_hot_container);
		if(this->block.gt_rle_container.n_entries)          zstd.encode(this->block.gt_rle_container);
		if(this->block.gt_simple_container.n_entries)       zstd.encode(this->block.gt_simple_container);
		if(this->block.gt_support_data_container.n_entries) zstd.encode(this->block.gt_support_data_container);
		if(this->block.meta_cold_container.n_entries)       zstd.encode(this->block.meta_cold_container);
		if(this->block.meta_info_map_ids.n_entries)         zstd.encode(this->block.meta_info_map_ids);
		if(this->block.meta_filter_map_ids.n_entries)       zstd.encode(this->block.meta_filter_map_ids);
		if(this->block.meta_format_map_ids.n_entries)       zstd.encode(this->block.meta_format_map_ids);

		for(U32 i = 0; i < this->block.index_entry.n_info_streams; ++i){
			if(!digests[this->header_->mapTable[this->block.index_entry.info_offsets[i].key]].updateUncompressed(this->block.info_containers[i])){
				std::cerr << Helpers::timestamp("ERROR","DIGEST") << "Failed to update digest..." << std::endl;
				return false;
			}

			zstd.encode(this->block.info_containers[i]);
		}

		for(U32 i = 0; i < this->block.index_entry.n_format_streams; ++i){
			if(!digests[this->header_->mapTable[this->block.index_entry.format_offsets[i].key]].updateUncompressed(this->block.format_containers[i])){
				std::cerr << Helpers::timestamp("ERROR","DIGEST") << "Failed to update digest..." << std::endl;
				return false;
			}

			zstd.encode(this->block.format_containers[i]);
		}

		// Perform writing
		this->block.updateOffsets();
		this->block.write(this->writer_.streamTomahawk, this->import_compressed_stats);

		// Reset and update
		this->resetHashes();
		this->block.clear();
		this->permutator.reset();
		this->writer_.streamTomahawk.flush();
		previousContigID = reader.first().body->CHROM;
		previousFirst    = reader.first().body->POS;
		previousLast     = reader.last().body->POS;
	}
	// Done importing
	std::cout << Helpers::timestamp("LOG","FINAL") << this->import_compressed_stats << '\t' << (U64)this->writer_.streamTomahawk.tellp() << std::endl;
	this->writer_.streamTomahawk.flush();
	const U64 data_ends = this->writer_.streamTomahawk.tellp();

	// Finalize SHA-512 digests
	const U64 digests_start = this->writer_.streamTomahawk.tellp();
	for(U32 i = 0; i < this->header_->map.size(); ++i){
		digests[i].finalize();
		this->writer_.streamTomahawk << digests[i];
		std::cerr << std::hex;
		for(U32 j = 0; j < 64; ++j)
			std::cerr << std::hex << (int)digests[i].sha512_digest[j];

		std::cerr << std::dec << std::endl;
	}
	delete [] digests;

	// Place markers
	this->writer_.streamTomahawk.write(reinterpret_cast<const char* const>(&digests_start), sizeof(U64));
	this->writer_.streamTomahawk.write(reinterpret_cast<const char* const>(&data_ends), sizeof(U64));

	// Write EOF
	BYTE eof_data[32];
	Helpers::HexToBytes(Constants::TACHYON_FILE_EOF, &eof_data[0]);
	this->writer_.streamTomahawk.write((char*)&eof_data[0], 32);
	this->writer_.streamTomahawk.flush();

	// All done
	return(true);
}

bool Importer::parseBCFLine(bcf_entry_type& entry){
	// Assert position is in range
	if(entry.body->POS + 1 > this->header_->getContig(entry.body->CHROM).bp_length){
		std::cerr << Helpers::timestamp("ERROR", "IMPORT") << this->header_->getContig(entry.body->CHROM).name << ':' << entry.body->POS+1 << " > reported max size of contig (" << this->header_->getContig(entry.body->CHROM).bp_length << ")..." << std::endl;
		return false;
	}

	// Perform run-length encoding
	U64 n_runs = 0;

	meta_type meta;
	meta.position          = entry.body->POS - this->block.index_entry.minPosition;
	meta.ref_alt           = entry.ref_alt;
	meta.controller.simple = entry.isSimple();

	// GT encoding
	if(!this->encoder.Encode(entry, meta, this->block.gt_rle_container, this->block.gt_simple_container, this->block.gt_support_data_container, n_runs, this->permutator.manager->get())){
		std::cerr << Helpers::timestamp("ERROR","ENCODER") << "Failed to encode GT..." << std::endl;
		return false;
	}

	if(!this->parseBCFBody(meta, entry)){
		std::cerr << Helpers::timestamp("ERROR","ENCODER") << "Failed to encode BCF body..." << std::endl;
		return false;
	}

	// RLE using this word size
	U32 w = ceil(ceil(log2(this->header_->samples + 1))/8);
	if((w > 2) & (w < 4)) w = 4;
	else if(w > 4) w = 8;

	// Complex meta data
	Core::MetaCold test;
	//meta.virtual_offset_cold_meta = this->block.meta_cold_container.buffer_data_uncompressed.pointer;
	if(!test.write(entry, this->block.meta_cold_container)){
		std::cerr << Helpers::timestamp("ERROR","ENCODER") << "Failed to write complex meta!" << std::endl;
		return false;
	}

	++this->block.meta_cold_container.n_entries;
	++this->block.meta_cold_container.n_additions;
	++this->block.meta_hot_container.n_entries;
	++this->block.meta_hot_container.n_additions;

	// Update number of entries in block
	++this->index_entry.n_variants;

	//meta.n_objects = n_runs;
	this->block.gt_support_data_container += (U32)n_runs;
	++this->block.gt_support_data_container;
	this->block.meta_hot_container.buffer_data_uncompressed += meta;

	return true;
}

bool Importer::parseBCFBody(meta_type& meta, bcf_entry_type& entry){
	U32 internal_pos = entry.filter_start;

	// At FILTER
	// Typed vector
	const bcf_entry_type::base_type& filter_key = *reinterpret_cast<const bcf_entry_type::base_type* const>(&entry.data[internal_pos++]);
	U32 n_filter = filter_key.high;
	if(n_filter == 15) n_filter = entry.getInteger(filter_key.low, internal_pos);
	entry.n_filter = n_filter;
	entry.filter_key = filter_key;

	S32 val = 0;
	while(entry.nextFilter(val, internal_pos)){
		// Hash FILTER value
		// Filter fields have no values
		this->filter_fields.setGet(val);
		//std::cerr << "FILTER: " << val << "->" << (*this->header_)[val].ID << std::endl;
	}

	// At INFO
	U32 info_length;
	BYTE info_value_type;
	while(entry.nextInfo(val, info_length, info_value_type, internal_pos)){
		// Hash INFO values
		const U32 mapID = this->info_fields.setGet(val);
		//std::cerr << Helpers::timestamp("DEBUG") << "Field: " << val << "->" << this->header_->mapTable[val] << "->" << this->header_->map[this->header_->mapTable[val]].ID << '\t' << mapID << '\t' << "length: " << info_length << '\t' <<internal_pos << "/" << (entry.body->l_shared + sizeof(U32)*2) << std::endl;

		stream_container& target_container = this->block.info_containers[mapID];
		if(this->block.info_containers[mapID].n_entries == 0){
			target_container.setStrideSize(info_length);
			target_container.header_stride.controller.type = Core::CORE_TYPE::TYPE_32B;
			target_container.header_stride.controller.signedness = 0;
			// Set all integer types to U32
			// Change to smaller type later if required
			if(info_value_type == 0)      target_container.setType(Core::CORE_TYPE::TYPE_32B);
			else if(info_value_type == 1) target_container.setType(Core::CORE_TYPE::TYPE_32B);
			else if(info_value_type == 2) target_container.setType(Core::CORE_TYPE::TYPE_32B);
			else if(info_value_type == 3) target_container.setType(Core::CORE_TYPE::TYPE_32B);
			else if(info_value_type == 5) target_container.setType(Core::CORE_TYPE::TYPE_FLOAT);
			else if(info_value_type == 7) target_container.setType(Core::CORE_TYPE::TYPE_CHAR);
			if(info_value_type != 5) target_container.header.controller.signedness = 1;
		}

		++target_container;
		if(!target_container.checkStrideSize(info_length))
			target_container.setMixedStrides();

		target_container.addStride(info_length);

		// Flags and integers
		// These are BCF value types
		if(info_value_type <= 3){
			for(U32 j = 0; j < info_length; ++j){
				target_container += entry.getInteger(info_value_type, internal_pos);
			}
		}
		// Floats
		else if(info_value_type == 5){
			for(U32 j = 0; j < info_length; ++j){
				target_container += entry.getFloat(internal_pos);
			}
		}
		// Chars
		else if(info_value_type == 7){
			for(U32 j = 0; j < info_length; ++j){
				target_container += entry.getChar(internal_pos);
			}
		}
		// Illegal: parsing error
		else {
			std::cerr << "impossible in info: " << (int)info_value_type << std::endl;
			exit(1);
		}
	}

#if IMPORT_ASSERT == 1
	// Assert all FILTER and INFO data have been successfully
	// parsed. This is true when the byte pointer equals the
	// start position of the FORMAT fields which are encoded
	// in the meta header structure
	assert(internal_pos == (entry.body->l_shared + sizeof(U32)*2));
#endif

	BYTE format_value_type = 0;
	while(entry.nextFormat(val, info_length, format_value_type, internal_pos)){
		const U32 mapID = this->format_fields.setGet(val);

		if(this->header_->map[this->header_->mapTable[val]].ID == "GT"){
			BYTE multiplier = sizeof(U32);
			switch(format_value_type){
				case(7):
				case(1): multiplier = sizeof(SBYTE); break;
				case(2): multiplier = sizeof(S16);   break;
				case(5):
				case(3): multiplier = sizeof(S32);   break;
				case(0): break; // FLAG
				default: std::cerr << "illegal" << std::endl; exit(1); return false;
			}
			internal_pos += this->header_->samples * info_length * multiplier;
			continue;
		}

		// Hash INFO values
		stream_container& target_container = this->block.format_containers[mapID];
		if(this->block.format_containers[mapID].n_entries == 0){
			target_container.setStrideSize(info_length);
			target_container.header_stride.controller.type = Core::CORE_TYPE::TYPE_32B;
			target_container.header_stride.controller.signedness = 0;
			// Set all integer types to U32
			// Change to smaller type later if required
			if(format_value_type == 0)      target_container.setType(Core::CORE_TYPE::TYPE_32B);
			else if(format_value_type == 1) target_container.setType(Core::CORE_TYPE::TYPE_32B);
			else if(format_value_type == 2) target_container.setType(Core::CORE_TYPE::TYPE_32B);
			else if(format_value_type == 3) target_container.setType(Core::CORE_TYPE::TYPE_32B);
			else if(format_value_type == 5) target_container.setType(Core::CORE_TYPE::TYPE_FLOAT);
			else if(format_value_type == 7) target_container.setType(Core::CORE_TYPE::TYPE_CHAR);
			else {
				std::cerr << "not possible" << std::endl;
				exit(1);
			}
			if(format_value_type != 5) target_container.header.controller.signedness = 1;
		}

		++target_container;
		if(!target_container.checkStrideSize(info_length))
			target_container.setMixedStrides();

		target_container.addStride(info_length);

		// Flags and integers
		// These are BCF value types
		if(format_value_type <= 3){
			for(U32 s = 0; s < this->header_->samples; ++s){
				//target_container.addStride(info_length);
				for(U32 j = 0; j < info_length; ++j){
					target_container += entry.getInteger(format_value_type, internal_pos);
				}
			}
		}
		// Floats
		else if(format_value_type == 5){
			for(U32 s = 0; s < this->header_->samples; ++s){
				//target_container.addStride(info_length);
				for(U32 j = 0; j < info_length; ++j){
					target_container += entry.getFloat(internal_pos);
				}
			}
		}
		// Chars
		else if(format_value_type == 7){
			for(U32 s = 0; s < this->header_->samples; ++s){
				//target_container.addStride(info_length);
				for(U32 j = 0; j < info_length; ++j){
					target_container += entry.getChar(internal_pos);
				}
			}
		}
		// Illegal: parsing error
		else {
			std::cerr << "impossible: " << (int)format_value_type << std::endl;
			std::cerr << Helpers::timestamp("LOG") << val << '\t' << info_length << '\t' << (int)format_value_type << '\t' << internal_pos << '/' << entry.pointer << std::endl;

			exit(1);
		}
	}
#if IMPORT_ASSERT == 1
	assert(internal_pos == entry.pointer);
#endif

	// Hash FILTER pattern
	const U64 hash_filter_vector = entry.hashFilter();

	U32 mapID = 0;
	if(this->filter_patterns.getRaw(hash_filter_vector, mapID)){

	} else {
		std::vector<U32> ret_pattern;
		for(U32 i = 0; i < entry.filterPointer; ++i)
			ret_pattern.push_back(entry.filterID[i]);

		mapID = this->filter_patterns.size();
		assert(mapID < 65536);
		this->filter_patterns.set(ret_pattern, hash_filter_vector);
	}

	// Store this map in the meta
	this->block.meta_filter_map_ids += mapID;
	//meta.FILTER_map_ID = mapID;

	// Hash INFO pattern
	const U64 hash_info_vector = entry.hashInfo();

	mapID = 0;
	if(this->info_patterns.getRaw(hash_info_vector, mapID)){

	} else {
		std::vector<U32> ret_pattern;
		for(U32 i = 0; i < entry.infoPointer; ++i)
			ret_pattern.push_back(entry.infoID[i]);

		mapID = this->info_patterns.size();
		assert(mapID < 65536);
		this->info_patterns.set(ret_pattern, hash_info_vector);
	}

	// Store this map in the meta
	//meta.INFO_map_ID = mapID;
	this->block.meta_info_map_ids += mapID;


	// Hash FORMAT pattern
	const U64 hash_format_vector = entry.hashFormat();

	mapID = 0;
	if(this->format_patterns.getRaw(hash_format_vector, mapID)){
	} else {
		std::vector<U32> ret_pattern;
		for(U32 i = 0; i < entry.formatPointer; ++i)
			ret_pattern.push_back(entry.formatID[i]);

		mapID = this->format_patterns.size();
		assert(mapID < 65536);
		this->format_patterns.set(ret_pattern, hash_format_vector);
	}

	// Store this map in the meta
	//meta.FORMAT_map_ID = mapID;
	this->block.meta_format_map_ids += mapID;

	// Update
	++this->block.meta_info_map_ids;
	++this->block.meta_format_map_ids;
	++this->block.meta_filter_map_ids;
	this->block.meta_info_map_ids.addStride((S32)1);
	this->block.meta_format_map_ids.addStride((S32)1);
	this->block.meta_filter_map_ids.addStride((S32)1);

	// Return
	return true;
}

} /* namespace Tomahawk */
