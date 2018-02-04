#ifndef MAGICCONSTANTS_H_
#define MAGICCONSTANTS_H_

#include <string>
#include "../support/type_definitions.h"

extern int SILENT;

namespace tachyon{
namespace constants{

extern std::string LITERAL_COMMAND_LINE;
extern std::string INTERPRETED_COMMAND;

/*------   Version   ------*/
const S32 TACHYON_VERSION_MAJOR    = 0;
const S32 TACHYON_VERSION_MINOR    = 2;
const S32 TACHYON_VERSION_RELEASE  = 0;
const S32 TACHYON_VERSION_NUMBER   = (TACHYON_VERSION_MAJOR *100*100 + TACHYON_VERSION_MINOR *100 + TACHYON_VERSION_RELEASE);
const std::string TACHYON_LIB_VERSION = std::to_string(TACHYON_VERSION_MAJOR) + '.' + std::to_string(TACHYON_VERSION_MINOR) + '.' + std::to_string(TACHYON_VERSION_RELEASE);

/*------   Basics   ------*/
const std::string PROGRAM_NAME = "tachyon";
const std::string OUTPUT_SUFFIX = "yon"; // yonder
const std::string FILE_HEADER = "TACHYON\7";

/*------   Genotype packing   ------*/
const BYTE ALLELE_PACK_WIDTH = 2; // bit / allele
const BYTE SNP_PACK_WIDTH = ALLELE_PACK_WIDTH * 2; // bits / genotype

// Encoding for alleles
const char ALLELE_LOOKUP[4] = {2, 3, 0, 1};
const char ALLELE_LOOKUP_REVERSE[4] = {'0', '1', '.', '?'};

/*------   Map 1- or 2-bit allele to genotypes   ------*/
// 0 -> 0
// 1 -> 1
// 2 -> 4
// 3 -> 5
const BYTE ALLELE_REDUCED_MAP[4] = {0, 1, 4, 5};
// 0 -> 0
// 1 -> 1
// 4 -> 4
// 5 -> 5
const BYTE ALLELE_SELF_MAP[6] = {0, 1, 0, 0, 4, 5};

/*------   Hot meta biallelic packing   ------*/
// Encoding for bases
const char* const REF_ALT_LOOKUP = "ATGCN";
const BYTE REF_ALT_A = 0;
const BYTE REF_ALT_T = 1;
const BYTE REF_ALT_G = 2;
const BYTE REF_ALT_C = 3;
const BYTE REF_ALT_N = 4;

const BYTE TRANSITION_MAP_BASE_A[5] = {0, 0, 1, 0, 0}; // A->G
const BYTE TRANSITION_MAP_BASE_T[5] = {0, 0, 0, 1, 0}; // T->C
const BYTE TRANSITION_MAP_BASE_G[5] = {1, 0, 0, 0, 0}; // G->A
const BYTE TRANSITION_MAP_BASE_C[5] = {0, 1, 0, 0, 0}; // C->T
const BYTE TRANSITION_MAP_BASE_N[5] = {0, 0, 0, 0, 0}; // None
const BYTE* const TRANSITION_MAP[5] = {TRANSITION_MAP_BASE_A, TRANSITION_MAP_BASE_T, TRANSITION_MAP_BASE_G, TRANSITION_MAP_BASE_C, TRANSITION_MAP_BASE_N};

const BYTE TRANSVERSION_MAP_BASE_A[5] = {0, 1, 0, 1, 0};
const BYTE TRANSVERSION_MAP_BASE_T[5] = {1, 0, 1, 0, 0};
const BYTE TRANSVERSION_MAP_BASE_G[5] = {0, 1, 0, 1, 0};
const BYTE TRANSVERSION_MAP_BASE_C[5] = {1, 0, 1, 0, 0};
const BYTE TRANSVERSION_MAP_BASE_N[5] = {0, 0, 0, 0, 0};
const BYTE* const TRANSVERSION_MAP[5] = {TRANSVERSION_MAP_BASE_A,TRANSVERSION_MAP_BASE_T, TRANSVERSION_MAP_BASE_G, TRANSVERSION_MAP_BASE_C, TRANSVERSION_MAP_BASE_N};

/*------   EOF markers   ------*/
// EOF markers
// 64-bit XXHASH of "TACHYON-BLOCK-EOF"
const U64 TACHYON_BLOCK_EOF = 7964708207515128046;
// 32b SHA-256 digest
// echo -n "TACHYON-EOF" | openssl dgst -sha256
const std::string TACHYON_FILE_EOF = "fa20427e118a1f0c7e1195e4f85e15e74368b112e70dd89fd02772e1d90cb5dd";

}
}

#endif /* MAGICCONSTANTS_H_ */
