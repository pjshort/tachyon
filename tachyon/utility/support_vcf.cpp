#include "support_vcf.h"

namespace tachyon{
namespace utility{

std::ostream& to_vcf_string(std::ostream& stream, containers::PrimitiveContainer<BYTE>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, containers::PrimitiveContainer<U16>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, containers::PrimitiveContainer<U32>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, containers::PrimitiveContainer<U64>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, containers::PrimitiveContainer<SBYTE>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	const BYTE* const ref = reinterpret_cast<const BYTE* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_BYTE_EOV)
		return(stream.put('.'));

	// First value
	if(ref[0] == YON_BYTE_MISSING) stream << '.';
	else stream << container[0];

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_BYTE_MISSING) stream << ",.";
		else if(ref[i] == YON_BYTE_EOV){ return stream; }
		else stream << ',' << container[i];
	}

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, containers::PrimitiveContainer<S16>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	const U16* const ref = reinterpret_cast<const U16* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_SHORT_EOV)
		return(stream.put('.'));

	// First value
	if(ref[0] == YON_SHORT_MISSING) stream << '.';
	else stream << container[0];

	// Remainder values
	for(U32 i = 1; i < container.size(); ++i){
		if(ref[i] == YON_SHORT_MISSING) stream << ",.";
		else if(ref[i] == YON_SHORT_EOV){ return stream; }
		else stream << ',' << container[i];
	}

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, containers::PrimitiveContainer<S32>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	const U32* const ref = reinterpret_cast<const U32* const>(container.data());

	// If the first value is end-of-vector then return
	if(ref[0] == YON_INT_EOV)
		return(stream.put('.'));

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
std::ostream& to_vcf_string_char(std::ostream& stream, containers::PrimitiveContainer<char>& container){
	if(container.size() == 0)
		return(stream.put('.'));

	stream << container[0];
	for(U32 i = 1; i < container.size(); ++i)
		stream << ',' << container[i];

	return(stream);
}

std::ostream& to_vcf_string(std::ostream& stream, containers::PrimitiveContainer<float>& container){
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


}
}