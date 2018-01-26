#include "VCFHeader.h"

namespace tachyon {
namespace vcf{

VCFHeader::VCFHeader() :
	error_bit(VCF_PASS),
	samples(0),
	version(0),
	mapTable(nullptr),
	contigsHashTable(nullptr),
	sampleHashTable(nullptr),
	map_lookup(nullptr)
{
}

VCFHeader::~VCFHeader(){
	delete this->contigsHashTable;
	delete this->sampleHashTable;
	delete this->map_lookup;
	delete [] this->mapTable;
}

bool VCFHeader::parse(reader_type& stream){
	if(!this->parseFirstLine(stream))
		return false;

	// Read remainder lines
	if(!this->parseHeaderLines(stream))
		return false;

	if(!this->buildContigTable())
		return false;

	// Copy string literal to header
	U32 curPos = stream.tellg();
	U32 headerLength = curPos - stream.size();
	this->literal.resize(headerLength);
	stream.stream_.seekg(0);
	stream.stream_.read(&this->literal[0], headerLength);
	stream.stream_.seekg(curPos);

	// Read samples line
	if(!this->parseSampleLine(stream))
		return false;

	return true;
}

bool VCFHeader::parse(const char* const data, const U32& length){
	U32 offset = 0;
	if(!this->parseFirstLine(data, offset))
		return false;

	// Read remainder lines
	if(!this->parseHeaderLines(data, offset))
		return false;

	if(!this->buildContigTable())
		return false;

	// Read samples line
	if(!this->parseSampleLine(data, offset, length))
		return false;

	for(U32 i = 0 ; i < this->lines.size(); ++i){
		if(this->lines[i].isIndexable == false)
			continue;

		// Todo: fix
		S32 idx = -1;
		for(U32 j = 0; j < this->lines[i].pairs.size(); ++j){
			//std::cerr << j << ':' << this->lines[i].pairs[j].KEY << "=" << this->lines[i].pairs[j].VALUE << std::endl;
			if(this->lines[i].pairs[j].KEY == "IDX"){
				idx = atoi(&this->lines[i].pairs[j].VALUE[0]);
			}
		}
		//std::cerr << "IDX in: " << idx << std::endl;
		assert(idx != -1);

		map_entry_type map_value(this->lines[i].pairs[0].VALUE, this->lines[i].type, idx);
		this->map.push_back(map_value);
	}

	std::sort(this->map.begin(), this->map.end());
	const S32 largest_idx = this->map.back().IDX;
	//for(U32 i = 0; i < this->map.size(); ++i)
	//	std::cerr << this->map[i].IDX << '\t' << this->map[i].ID << std::endl;

	//std::cerr << "largest: " << largest_idx << std::endl;
	this->mapTable = new U32[largest_idx + 1];
	memset(this->mapTable, 0, sizeof(U32)*(largest_idx+1));
	S32 localID = 0;
	for(U32 i = 0; i < this->map.size(); ++i){
		mapTable[this->map[i].IDX] = localID++;
		//std::cerr << i << "->" << mapTable[i] << std::endl;
	}

	if(this->map.size()*2 < 1024)
		this->map_lookup = new hash_table_map_type(1024);
	else this->map_lookup = new hash_table_map_type(this->map.size()*2);

	for(U32 i = 0 ; i < this->lines.size(); ++i){
		//this->map_lookup->SetItem(&i, this->map[i]);
	}

	return true;
}

void VCFHeader::buildSampleTable(U64 samples){
	this->samples = samples;
	delete this->sampleHashTable;

	if(this->samples*2 < 1024)
		this->sampleHashTable = new hash_table_type(1024);
	else
		this->sampleHashTable = new hash_table_type(this->samples * 2);
}

bool VCFHeader::checkLine(const char* data, const U32 length){
	header_line_type line(data, length);
	if(line.Parse()){
		// If the line is a contig line: make sure it is legal
		// for our purposes
		if(line.isCONTIG()){
			contig_type contig;
			BYTE found = 0;

			// Contig line has two values:
			// ID: contig name
			// length: for length is bp
			for(U32 i = 0; i < line.size(); ++i){
				if(strncasecmp(&line[i].KEY[0], "ID", 2) == 0 && line[i].KEY.size() == 2){
					contig.name = line[i].VALUE;
					++found;
				} else if(strncasecmp(&line[i].KEY[0], "length", 6) == 0 && line[i].KEY.size() == 6){
					contig.bp_length = atoi(&line[i].VALUE[0]);
					++found;
				}
			}

			// Throw error if this pattern is not found
			if(found != 2){
				std::cerr << helpers::timestamp("WARNING","VCF") << "Illegal contig entry line with no length defined!" << std::endl;
				std::cerr << helpers::timestamp("WARNING","VCF") << "Offending line: " << std::string(data, length+1) << std::endl;
				contig.bp_length = std::numeric_limits<U32>::max();
			}
			this->contigs.push_back(contig);
		}

		// Also store the line as an object
		// and as a literal string as observed
		// in the VCF header
		this->lines.push_back(line); // parseable lines
		this->literal_lines.push_back(std::string(data, length + 1));
		return true;
	}

	std::cerr << helpers::timestamp("ERROR","VCF") << "Failed to parse VCF..." << std::endl;
	return false;
}

bool VCFHeader::buildContigTable(void){
	S32* retValue;

	delete this->contigsHashTable;

	if(this->contigs.size() < 1024)
		this->contigsHashTable = new hash_table_type(1024);
	else
		this->contigsHashTable = new hash_table_type(this->contigs.size() * 2);

	if(!SILENT)
		std::cerr << helpers::timestamp("LOG", "VCF") << "Constructing lookup table for " << this->contigs.size() << " contigs..." << std::endl;

	for(U32 i = 0; i < this->contigs.size(); ++i){
		if(!(*this).getContig(this->contigs[i].name, retValue)){
			(*this).addContig(this->contigs[i].name, i);
		} else {
			std::cerr << helpers::timestamp("ERROR", "VCF") << "Duplicated contig found (" << this->getContig(*retValue).name << "). Illegal..." << std::endl;
			this->error_bit = VCF_ERROR_LINES;
			return false;
		}
	}
	return true;
}

bool VCFHeader::parseFirstLine(reader_type& stream){
	if(!stream.good()){
		this->error_bit = STREAM_BAD;
		return false;
	}

	if(!stream.getLine()){
		std::cerr << helpers::timestamp("ERROR", "VCF") << "Could not validate file..." << std::endl;
		this->error_bit = STREAM_BAD;
		return false;
	}

	// Parse
	if(strncmp(&stream[0], &vcf::constants::HEADER_VCF_FORMAT[0], vcf::constants::HEADER_VCF_FORMAT.size()) != 0){
		std::cerr << helpers::timestamp("ERROR", "VCF") << "Invalid VCF format..." << std::endl;
		this->error_bit = VCF_ERROR_LINE1;
		return false;
	}

	if(strncmp(&stream[0], &vcf::constants::HEADER_VCF_VERSION[0], vcf::constants::HEADER_VCF_VERSION.size()) != 0){
		std::cerr << helpers::timestamp("ERROR", "VCF") << "Invalid VCF version < 4.x..." << std::endl;
		this->error_bit = VCF_ERROR_LINE1;
		return false;
	}

	this->literal_lines.push_back(std::string(&stream[0],stream.size()-1));
	stream.clear();
	return true;
}

bool VCFHeader::parseFirstLine(const char* const data, U32& offset){
	offset = 4;
	if(strncmp(&data[offset], &vcf::constants::HEADER_VCF_FORMAT[0], vcf::constants::HEADER_VCF_FORMAT.size()) != 0){
		std::cerr << helpers::timestamp("ERROR", "BCF") << "Invalid VCF format..." << std::endl;
		std::cerr << std::string(&data[offset], 100) << std::endl;
		this->error_bit = VCF_ERROR_LINE1;
		return false;
	}

	if(strncmp(&data[offset], &vcf::constants::HEADER_VCF_VERSION[0], vcf::constants::HEADER_VCF_VERSION.size()) != 0){
		std::cerr << helpers::timestamp("ERROR", "BCF") << "Invalid VCF version < 4.x..." << std::endl;
		this->error_bit = VCF_ERROR_LINE1;
		return false;
	}

	const char* hit = std::strchr(&data[offset], '\n');
	this->literal_lines.push_back(std::string(&data[offset], hit - &data[offset]));
	offset += (hit + 1) - &data[offset];
	return true;
}

bool VCFHeader::parseHeaderLines(reader_type& stream){
	while(stream.getLine()){
		if(stream.buffer_[1] != '#')
			break;

		if(!this->checkLine(stream.buffer_, stream.size() - 2)){
			std::cerr << helpers::timestamp("ERROR", "VCF") << "Failed to validate header lines" << std::endl;
			this->error_bit = VCF_ERROR_LINES;
			return false;
		}

		stream.clear();
	}

	if(!stream.good()){
		this->error_bit = STREAM_BAD;
		return false;
	}

	return true;
}

bool VCFHeader::parseHeaderLines(const char* const data, U32& offset){
	std::istringstream is(&data[offset]);
	std::string line;
	while(std::getline(is, line)){
		if(line[1] != '#')
			break;

		if(!this->checkLine(&line[0], line.size() - 1)){
			std::cerr << helpers::timestamp("ERROR", "BCF") << "Failed to validate header lines" << std::endl;
			this->error_bit = VCF_ERROR_LINES;
			return false;
		}
	}

	offset += (U32)is.tellg() - line.size() - 1;

	return true;
}

bool VCFHeader::parseSampleLine(reader_type& stream){
	// At broken position is main header line
	// Validate header
	if(strncmp(&vcf::constants::HEADER_COLUMN[0], &stream.buffer_[0], vcf::constants::HEADER_COLUMN.size()) != 0){
		std::cerr << helpers::timestamp("ERROR", "VCF") << "Could not validate header line" << std::endl;
		this->error_bit = VCF_ERROR_SAMPLE;
		return false;
	}

	U32 search_position = vcf::constants::HEADER_COLUMN.size() + 1;
	U64 delimiters_found = 0;
	while(true){ // while there is samples in line
		char* found = std::find(&stream[search_position], &stream[stream.size()-1], vcf::constants::VCF_DELIMITER);
		if(*found != vcf::constants::VCF_DELIMITER)
			break;

		//std::cerr << std::string(&stream[search_position], (found - stream.buffer_ + 1) - search_position) << std::endl;
		search_position = found - stream.buffer_ + 1;
		++delimiters_found;
	}

	this->buildSampleTable(delimiters_found);

	// Parse
	search_position = vcf::constants::HEADER_COLUMN.size() + 1;
	delimiters_found = 0;
	S32* retValue;
	char* found = 0;
	while(found != &stream[stream.size()-1]){ // while there are samples in line
		found = std::find(&stream[search_position], &stream[stream.size()-1], vcf::constants::VCF_DELIMITER);
		std::string sampleName(&stream[search_position], (found - stream.buffer_ + 1) - search_position - 1);

		if(sampleName == "FORMAT"){
			search_position = found - stream.buffer_ + 1;
			continue;
		}

		if(!this->getSample(sampleName, retValue)) this->addSample(sampleName);
		else {
			std::cerr << helpers::timestamp("ERROR", "VCF") << "Duplicated sample name in header..." << std::endl;
			this->error_bit = VCF_ERROR_LINES;
		}

		search_position = found - stream.buffer_ + 1;
	}

	stream.clear();
	return true;
}

bool VCFHeader::parseSampleLine(const char* const data, U32& offset, const U32& length){
	// At broken position is main header line
	// Validate header
	if(strncmp(&vcf::constants::HEADER_COLUMN[0], &data[offset], vcf::constants::HEADER_COLUMN.size()) != 0){
		std::cerr << helpers::timestamp("ERROR", "VCF") << "Could not validate header line" << std::endl;
		this->error_bit = VCF_ERROR_SAMPLE;
		return false;
	}

	offset += vcf::constants::HEADER_COLUMN.size() + 1;
	U64 delimiters_found = 0;
	U32 offset_original = offset;

	while(true){ // while there is samples in line
		const char* const found = std::strchr(&data[offset], vcf::constants::VCF_DELIMITER);
		//std::cerr << (void*)found << '\t' << (void*)&data[length] << std::endl;
		if(found == 0 || (*found != vcf::constants::VCF_DELIMITER)){
			std::string sampleName(&data[offset], (&data[length - 1] - &data[offset]) - 1); // -2 because offset is +1 and newline is +1
			//std::cerr << sampleName << std::endl;
			++delimiters_found;
			break;
		}

		std::string sampleName(&data[offset], (found - &data[offset]));
		if(sampleName == "FORMAT"){
			offset += found - &data[offset] + 1;
			continue;
		}

		offset += found - &data[offset] + 1;
		++delimiters_found;
	}

	this->buildSampleTable(delimiters_found);

	offset = offset_original;
	S32* retValue;
	while(true){ // while there is samples in line
		const char* const found = std::strchr(&data[offset], vcf::constants::VCF_DELIMITER);
		if(found == 0 || (*found != vcf::constants::VCF_DELIMITER)){
			std::string sampleName(&data[offset], (&data[length - 1] - &data[offset]) - 1); // -2 because offset is +1 and newline is +1
			if(!this->getSample(sampleName, retValue))
				this->addSample(sampleName);
			else {
				std::cerr << helpers::timestamp("ERROR", "VCF") << "Duplicated sample name in header..." << std::endl;
				this->error_bit = VCF_ERROR_LINES;
			}
			break;
		}


		std::string sampleName(&data[offset], (found - &data[offset]));
		if(sampleName == "FORMAT"){
			offset += found - &data[offset] + 1;
			continue;
		}

		if(!this->getSample(sampleName, retValue))
			this->addSample(sampleName);
		else {
			std::cerr << helpers::timestamp("ERROR", "VCF") << "Duplicated sample name in header..." << std::endl;
			this->error_bit = VCF_ERROR_LINES;
		}
		offset += found - &data[offset] + 1;
	}

	return true;
}


}
} /* namespace Tachyon */