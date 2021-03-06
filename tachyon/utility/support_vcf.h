#ifndef UTILITY_SUPPORT_VCF_H_
#define UTILITY_SUPPORT_VCF_H_


#include <iostream>
#include <cmath>
#include "../support/type_definitions.h"
#include "../containers/primitive_container.h"
#include "../core/genotype_object.h"

namespace tachyon{
namespace utility{

#define YON_BYTE_MISSING        0x80
#define YON_BYTE_EOV            0x81
#define YON_SHORT_MISSING       0x8000
#define YON_SHORT_EOV           0x8001
#define YON_INT_MISSING         0x80000000
#define YON_INT_EOV             0x80000001
#define YON_FLOAT_NAN           0x7FC00000
#define YON_FLOAT_MISSING       0x7F800001
#define YON_FLOAT_EOV           0x7F800002

// Base functionality converting data to a valid VCF string
std::ostream& to_vcf_string(std::ostream& stream, const BYTE* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const U16* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const U32* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const U64* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const SBYTE* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const S16* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const S32* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const char* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const float* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const double* const values, const U32 n_entries);
std::ostream& to_vcf_string(std::ostream& stream, const std::string& string);

// Primitive container declarations
// Unsigned values does not have missing values
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<BYTE>& container);
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<U16>& container);
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<U32>& container);
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<U64>& container);
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<SBYTE>& container);
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<S16>& container);
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<S32>& container);
std::ostream& to_vcf_string_char(std::ostream& stream, const containers::PrimitiveContainer<char>& container);
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<float>& container);
std::ostream& to_vcf_string(std::ostream& stream, const containers::PrimitiveContainer<double>& container);

io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<BYTE>& container);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U16>& container);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U32>& container);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U64>& container);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<SBYTE>& container);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<S16>& container);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<S32>& container);
io::BasicBuffer& to_vcf_string_char(io::BasicBuffer& buffer, const containers::PrimitiveContainer<char>& container);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<float>& container);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<double>& container);

io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<BYTE>& container);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U16>& container);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U32>& container);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<U64>& container);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<SBYTE>& container);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<S16>& container);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<S32>& container);
io::BasicBuffer& to_json_string_char(io::BasicBuffer& buffer, const containers::PrimitiveContainer<char>& container);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<float>& container);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const containers::PrimitiveContainer<double>& container);

// Genotype objects
std::ostream& to_vcf_string(std::ostream& stream, const core::GTObject& gt_object);
std::ostream& to_vcf_string(std::ostream& stream, const std::vector<core::GTObject>& gt_objects);

std::ostream& to_vcf_string(std::ostream& stream, const char& delimiter, const core::MetaEntry& meta_entry, const core::VariantHeader& header);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const char& delimiter, const core::MetaEntry& meta_entry, const core::VariantHeader& header);
io::BasicBuffer& to_vcf_string(io::BasicBuffer& buffer, const char& delimiter, const core::MetaEntry& meta_entry, const core::VariantHeader& header, const core::SettingsCustomOutput& controller);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const core::MetaEntry& meta_entry, const core::VariantHeader& header, const core::SettingsCustomOutput& controller);
io::BasicBuffer& to_json_string(io::BasicBuffer& buffer, const char& delimiter, const core::MetaEntry& meta_entry, const core::VariantHeader& header, const core::SettingsCustomOutput& controller);

}
}



#endif /* UTILITY_SUPPORT_VCF_H_ */
