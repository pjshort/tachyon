#include "support_vcf.h"

namespace tachyon{
namespace utility{

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<BYTE>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << (U32)container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<U16>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << (U32)container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<U32>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<U64>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<SBYTE>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	const BYTE* const ref = reinterpret_cast<const BYTE* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_BYTE_EOV)
		return(stream.put('.'));

	// First value
	if(ref[0] == YON_BYTE_MISSING) stream << '.';
	else stream << (S32)container[0];

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_BYTE_MISSING) stream << ",.";
		else if(ref[i] == YON_BYTE_EOV){ return stream; }
		else stream << ',' << (S32)container[i];
	}

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<S16>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	const U16* const ref = reinterpret_cast<const U16* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_SHORT_EOV)
		return(stream.put('.'));

	// First value
	if(ref[0] == YON_SHORT_MISSING) stream << '.';
	else stream << (S32)container[0];

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_SHORT_MISSING) stream << ",.";
		else if(ref[i] == YON_SHORT_EOV){ return stream; }
		else stream << ',' << (S32)container[i];
	}

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<S32>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_INT_EOV){
		return(stream.put('.'));
	}

	// First value
	if(ref[0] == YON_INT_MISSING) stream << '.';
	else stream << container[0];

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_INT_MISSING) stream << ",.";
		else if(ref[i] == YON_INT_EOV){ return stream; }
		else stream << ',' << container[i];
	}

	return(stream);
}

// Special case
std::ostream& to_vcf_string_char(std::ostream& stream, const containers::PrimitiveContainer<char>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<float>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_FLOAT_EOV)
		return(stream.put('.'));

	// First value
	if(ref[0] == YON_FLOAT_MISSING) stream << '.';
	else stream << container[0];

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_FLOAT_MISSING) stream << ",.";
		else if(ref[i] == YON_FLOAT_EOV){ return stream; }
		else stream << ',' << container[i];
	}

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<double>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_FLOAT_EOV)
		return(stream.put('.'));

	// First value
	if(ref[0] == YON_FLOAT_MISSING) stream << '.';
	else stream << container[0];

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_FLOAT_MISSING) stream << ",.";
		else if(ref[i] == YON_FLOAT_EOV){ return stream; }
		else stream << ',' << container[i];
	}

	return(stream);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<BYTE>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	buffer.AddReadble((U32)container[0]);
	for(U32 i = 1; i < container.size(); ++i){
		buffer += ',';
		buffer.AddReadble((U32)container[i]);
	}

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U16>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	buffer.AddReadble(container[0]);
	for(U32 i = 1; i < container.size(); ++i){
		buffer += ',';
		buffer.AddReadble(container[i]);
	}

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U32>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	buffer.AddReadble(container[0]);
	for(U32 i = 1; i < container.size(); ++i){
		buffer += ',';
		buffer.AddReadble(container[i]);
	}

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U64>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	buffer.AddReadble(container[0]);
	for(U32 i = 1; i < container.size(); ++i){
		buffer += ',';
		buffer.AddReadble(container[i]);
	}

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<SBYTE>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	const BYTE* const ref = reinterpret_cast<const BYTE* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_BYTE_EOV){
		buffer += '.';
		return(buffer);
	}

	// First value
	if(ref[0] == YON_BYTE_MISSING) buffer += '.';
	else buffer.AddReadble((S32)container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_BYTE_MISSING) buffer += ",.";
		else if(ref[i] == YON_BYTE_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble((S32)container[i]);
		}
	}

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<S16>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	const U16* const ref = reinterpret_cast<const U16* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_SHORT_EOV){
		buffer += '.';
		return(buffer);
	}

	// First value
	if(ref[0] == YON_SHORT_MISSING) buffer += '.';
	else buffer.AddReadble((S32)container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_SHORT_MISSING) buffer += ",.";
		else if(ref[i] == YON_SHORT_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble((S32)container[i]);
		}
	}

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<S32>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_INT_EOV){
		buffer += '.';
		return(buffer);
	}

	// First value
	if(ref[0] == YON_INT_MISSING) buffer += '.';
	else buffer.AddReadble(container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_INT_MISSING) buffer += ",.";
		else if(ref[i] == YON_INT_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble(container[i]);
		}
	}

	return(buffer);
}

// Special case
io::BasicBuffer& to_vcf_string_char(io::BasicBuffer& buffer, const containers::PrimitiveContainer<char>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	buffer += container[0];
	for(U32 i = 1; i < container.size(); ++i){
		buffer += ',';
		buffer += container[i];
	}

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<float>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_FLOAT_EOV){
		buffer += '.';
		return(buffer);
	}

	// First value
	if(ref[0] == YON_FLOAT_MISSING) buffer += '.';
	else buffer.AddReadble(container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_FLOAT_MISSING) buffer += ",.";
		else if(ref[i] == YON_FLOAT_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble(container[i]);
		}
	}

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<double>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_FLOAT_EOV){
		buffer += '.';
		return(buffer);
	}

	// First value
	if(ref[0] == YON_FLOAT_MISSING) buffer += '.';
	else buffer.AddReadble(container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_FLOAT_MISSING) buffer += ",.";
		else if(ref[i] == YON_FLOAT_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble(container[i]);
		}
	}

	return(buffer);
}

///////////
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<BYTE>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	if(container.size() == 1){
		buffer.AddReadble((U32)container[0]);
	} else {
		buffer += '[';
		buffer.AddReadble((U32)container[0]);
		for(U32 i = 1; i < container.size(); ++i){
			buffer += ',';
			buffer.AddReadble((U32)container[i]);
		}
		buffer += ']';
	}

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U16>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	if(container.size() == 1){
		buffer.AddReadble(container[0]);
	} else {
		buffer += '[';
		buffer.AddReadble(container[0]);
		for(U32 i = 1; i < container.size(); ++i){
			buffer += ',';
			buffer.AddReadble(container[i]);
		}
		buffer += ']';
	}

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U32>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	if(container.size() == 1){
		buffer.AddReadble(container[0]);
	} else {
		buffer += '[';
		buffer.AddReadble(container[0]);
		for(U32 i = 1; i < container.size(); ++i){
			buffer += ',';
			buffer.AddReadble(container[i]);
		}
		buffer += ']';
	}

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U64>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	if(container.size() == 1){
		buffer.AddReadble(container[0]);
	} else {
		buffer += '[';
		buffer.AddReadble(container[0]);
		for(U32 i = 1; i < container.size(); ++i){
			buffer += ',';
			buffer.AddReadble(container[i]);
		}
		buffer += ']';
	}

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<SBYTE>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	const BYTE* const ref = reinterpret_cast<const BYTE* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_BYTE_EOV){
		buffer += "null";
		return(buffer);
	}

	// First value
	if(container.size() == 1){
		if(ref[0] == YON_BYTE_MISSING) buffer += "null";
		else buffer.AddReadble((S32)container[0]);
		return(buffer);
	}

	buffer += '[';
	if(ref[0] == YON_BYTE_MISSING) buffer += "null";
	else buffer.AddReadble((S32)container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_BYTE_MISSING) buffer += ",null";
		else if(ref[i] == YON_BYTE_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble((S32)container[i]);
		}
	}
	buffer += ']';

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<S16>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	const U16* const ref = reinterpret_cast<const U16* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_SHORT_EOV){
		buffer += "null";
		return(buffer);
	}

	// First value
	if(container.size() == 1){
		if(ref[0] == YON_SHORT_MISSING) buffer += "null";
		else buffer.AddReadble((S32)container[0]);
		return(buffer);
	}

	buffer += '[';
	if(ref[0] == YON_SHORT_MISSING) buffer += "null";
	else buffer.AddReadble((S32)container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_SHORT_MISSING) buffer += ",null";
		else if(ref[i] == YON_SHORT_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble((S32)container[i]);
		}
	}
	buffer += ']';

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<S32>& container){
	if(container.size() == 0){
		buffer += '.';
		return(buffer);
	}

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_INT_EOV){
		buffer += "null";
		return(buffer);
	}

	// First value
	if(container.size() == 1){
		if(ref[0] == YON_INT_MISSING) buffer += "null";
		else buffer.AddReadble((S32)container[0]);
		return(buffer);
	}

	buffer += '[';
	if(ref[0] == YON_INT_MISSING) buffer += "null";
	else buffer.AddReadble((S32)container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_INT_MISSING) buffer += ",null";
		else if(ref[i] == YON_INT_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble((S32)container[i]);
		}
	}
	buffer += ']';

	return(buffer);
}

// Special case
io::BasicBuffer& to_json_string_char(io::BasicBuffer& buffer, const containers::PrimitiveContainer<char>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	if(container.size() == 1){
		buffer += container[0];
		return(buffer);
	}

	buffer += '[';
	buffer += container[0];
	for(U32 i = 1; i < container.size(); ++i){
		buffer += ',';
		buffer += container[i];
	}
	buffer += ']';

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<float>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_FLOAT_EOV){
		buffer += "null";
		return(buffer);
	}

	// First value
	if(container.size() == 1){
		if(ref[0] == YON_FLOAT_MISSING) buffer += "null";
		else buffer.AddReadble(container[0]);
		return(buffer);
	}

	buffer += '[';
	if(ref[0] == YON_FLOAT_MISSING) buffer += "null";
	else buffer.AddReadble(container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_FLOAT_MISSING) buffer += ",null";
		else if(ref[i] == YON_FLOAT_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble(container[i]);
		}
	}
	buffer += ']';

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<double>& container){
	if(container.size() == 0){
		buffer += "null";
		return(buffer);
	}

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_FLOAT_EOV){
		buffer += "null";
		return(buffer);
	}

	// First value
	if(container.size() == 1){
		if(ref[0] == YON_FLOAT_MISSING) buffer += "null";
		else buffer.AddReadble(container[0]);
		return(buffer);
	}

	buffer += '[';
	if(ref[0] == YON_FLOAT_MISSING) buffer += "null";
	else buffer.AddReadble(container[0]);

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_FLOAT_MISSING) buffer += ",null";
		else if(ref[i] == YON_FLOAT_EOV){ return buffer; }
		else {
			buffer += ',';
			buffer.AddReadble(container[i]);
		}
	}
	buffer += ']';

	return(buffer);
}

///

std::ostream& to_vcf_string(std::ostream& stream, const core::GTObject& gt_object){
	if(gt_object.n_alleles == 0)
		return(stream);

	for(U32 i = 0; i < gt_object.n_objects; ++i){
		stream << (int)gt_object[0].first << (gt_object[i].second ? '|' : '/') << (int)gt_object[1].first;
		for(U32 j = 1; j < gt_object.n_alleles; ++j){
			stream << '\t' << (int)gt_object[0].first << (gt_object[0].second ? '|' : '/') << (int)gt_object[1].first;
		}
	}
	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const std::vector<core::GTObject>& gt_objects){
	if(gt_objects.size() == 0)
		return(stream);

	stream << (int)gt_objects[0][0].first << (gt_objects[0][0].second ? '|' : '/') << (int)gt_objects[0][1].first;

	for(U32 element = 1; element < gt_objects.size(); ++element){
		if(gt_objects[element].n_alleles == 0)
			continue;

		stream << '\t';

		for(U32 object = 0; object < gt_objects[element].n_objects; ++object){
			stream << (int)gt_objects[element][0].first << (gt_objects[element][0].second ? '|' : '/') << (int)gt_objects[element][1].first;
			for(U32 k = 1; k < gt_objects[element].n_alleles; ++k){
				stream << '\t' << (int)gt_objects[element][k].first << (gt_objects[element][k].second ? '|' : '/') << (int)gt_objects[element][k].first;
			}
		}
	}
	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const std::string& string){
	stream << string;
	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, const char& delimiter, const core::MetaEntry& meta_entry, const core::VariantHeader& header){
	stream.write(&header.getContig(meta_entry.contigID).name[0], header.getContig(meta_entry.contigID).name.size()) << '\t';
	stream << meta_entry.position + 1 << delimiter;

	if(meta_entry.name.size() == 0) stream.put('.');
	else stream.write(&meta_entry.name[0], meta_entry.name.size());
	stream.put(delimiter);
	if(meta_entry.n_alleles){
		stream.write(meta_entry.alleles[0].allele, meta_entry.alleles[0].l_allele);
		//stream << meta_entry.alleles[0].l_allele;
		stream.put(delimiter);
		stream.write(meta_entry.alleles[1].allele, meta_entry.alleles[1].l_allele);
		for(U32 i = 2; i < meta_entry.n_alleles; ++i){
			stream.put(',');
			stream.write(meta_entry.alleles[i].allele, meta_entry.alleles[i].l_allele);
		}
	} else {
		//stream << ".\t.\t";
		stream << '.' << delimiter << '.' << delimiter;
	}

	if(std::isnan(meta_entry.quality)){
		stream << delimiter << '.' << delimiter;
		//stream << "\t.\t";
	}
	else {
		stream << delimiter << meta_entry.quality << delimiter;
		//stream << '\t' << meta_entry.quality << '\t';
	}
	return(stream);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const char& delimiter, const core::MetaEntry& meta_entry, const core::VariantHeader& header){
	buffer += header.getContig(meta_entry.contigID).name;
	buffer += delimiter;
	buffer.AddReadble(meta_entry.position + 1);
	buffer += delimiter;

	if(meta_entry.name.size() == 0) buffer += '.';
	else buffer += meta_entry.name;
	buffer += delimiter;
	if(meta_entry.n_alleles){
		buffer.Add(meta_entry.alleles[0].allele, meta_entry.alleles[0].l_allele);
		buffer += delimiter;
		buffer.Add(meta_entry.alleles[1].allele, meta_entry.alleles[1].l_allele);
		for(U32 i = 2; i < meta_entry.n_alleles; ++i){
			buffer += ',';
			buffer.Add(meta_entry.alleles[i].allele, meta_entry.alleles[i].l_allele);
		}
		buffer += delimiter;
	} else {
		buffer += '.';
		buffer += delimiter;
		buffer += '.';
		buffer += delimiter;
	}

	if(std::isnan(meta_entry.quality)){
		buffer += '.';
	}
	else {
		buffer.AddReadble(meta_entry.quality);
	}
	buffer += delimiter;

	return(buffer);
}

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const char& delimiter, const core::MetaEntry& meta_entry, const core::VariantHeader& header, const core::SettingsCustomOutput& controller){
	if(controller.show_contig){
		buffer += header.getContig(meta_entry.contigID).name;
		buffer += delimiter;
	}

	if(controller.show_position){
		buffer.AddReadble(meta_entry.position + 1);
		buffer += delimiter;
	}

	if(controller.show_names){
		if(meta_entry.name.size() == 0) buffer += '.';
		else buffer += meta_entry.name;
		buffer += delimiter;
	}

	if(controller.show_ref){
		if(meta_entry.n_alleles) buffer.Add(meta_entry.alleles[0].allele, meta_entry.alleles[0].l_allele);
		else buffer += '.';
		buffer += delimiter;
	}

	if(controller.show_alt){
		if(meta_entry.n_alleles){
			buffer.Add(meta_entry.alleles[1].allele, meta_entry.alleles[1].l_allele);
			for(U32 i = 2; i < meta_entry.n_alleles; ++i){
				buffer += ',';
				buffer.Add(meta_entry.alleles[i].allele, meta_entry.alleles[i].l_allele);
			}
		} else
			buffer += '.';

		buffer += delimiter;
	}

	if(controller.show_quality){
		if(std::isnan(meta_entry.quality)) buffer += '.';
		else buffer.AddReadble(meta_entry.quality);
		buffer += delimiter;
	}

	return(buffer);
}

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const core::MetaEntry& meta_entry, const core::VariantHeader& header, const core::SettingsCustomOutput& controller){
	return(utility::to_json_string(buffer, meta_entry, header, controller));
}


io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const char& delimiter, const core::MetaEntry& meta_entry, const core::VariantHeader& header, const core::SettingsCustomOutput& controller){
	bool add = false;
	if(controller.show_contig){
		buffer += "\"contig\":\"";
		buffer += header.getContig(meta_entry.contigID).name;
		buffer += '"';
		add = true;
	}

	if(controller.show_position){
		if(add){ buffer += ','; add = false; }
		buffer += "\"position\":";
		buffer.AddReadble(meta_entry.position + 1);
		add = true;
	}

	if(controller.show_names){
		if(add){ buffer += ','; add = false; }
		buffer += "\"name\":";
		if(meta_entry.name.size() == 0) buffer += "null";
		else {
			buffer += '"';
			buffer += meta_entry.name;
			buffer += '"';
			add = true;
		}
	}

	if(controller.show_ref){
		if(add){ buffer += ','; add = false; }
		buffer += "\"ref\":";
		if(meta_entry.n_alleles){
			buffer += '"';
			buffer.Add(meta_entry.alleles[0].allele, meta_entry.alleles[0].l_allele);
			buffer += '"';
		} else {
			buffer += "null";
		}
		add = true;
	}

	if(controller.show_alt){
		if(add){ buffer += ','; add = false; }
		buffer += "\"alt\":";
		if(meta_entry.n_alleles){
			if(meta_entry.n_alleles > 2) buffer += '[';
			buffer += '"';
			buffer.Add(meta_entry.alleles[1].allele, meta_entry.alleles[1].l_allele);
			buffer += '"';
			for(U32 i = 2; i < meta_entry.n_alleles; ++i){
				buffer += ',';
				buffer += '"';
				buffer.Add(meta_entry.alleles[i].allele, meta_entry.alleles[i].l_allele);
				buffer += '"';
			}
			if(meta_entry.n_alleles > 2) buffer += ']';
		} else {
			buffer += "null";
		}
		add = true;
	}

	if(controller.show_quality){
		if(add){ buffer += ','; add = false; }
		buffer += "\"quality\":";
		if(std::isnan(meta_entry.quality)) buffer += 0;
		else buffer.AddReadble(meta_entry.quality);
		add = true;
	}

	return(buffer);
}

}
}
