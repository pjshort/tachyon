#ifndef CORE_BASE_ENTRYCOLDMETA_H_
#define CORE_BASE_ENTRYCOLDMETA_H_

#include <cassert>

#include "../../io/bcf/BCFEntry.h"
#include "../StreamContainer.h"

namespace Tachyon{
namespace Core{

/** ColdMetaAllele:
 *  @brief Contains parts of the cold component of the hot-cold split of a variant site meta information
 *  This is a supportive structure. It keeps allele information
 *  as a typed string. This data structure is always cast
 *  directly from pre-loaded byte streams.
 */
struct ColdMetaAllele{
private:
	typedef ColdMetaAllele self_type;
	typedef IO::BasicBuffer buffer_type;

public:
	ColdMetaAllele() : l_allele(0), allele(nullptr){}
	ColdMetaAllele(char* in) :
		l_allele(*reinterpret_cast<U16*>(&in[0])),
		allele(&in[sizeof(U16)])
	{}
	~ColdMetaAllele(void){} // do nothing

	inline const U32 size(void) const{ return(this->l_allele + sizeof(U16)); }

	friend buffer_type& operator+=(buffer_type& buffer, const self_type& entry){
		// Write out allele
		buffer += (U16)entry.l_allele;
		buffer.Add(entry.allele, entry.l_allele);

		return(buffer);
	}

public:
	U16 l_allele; /**< Byte length of allele data */
	char* allele; /**< Char array of allele */
};

// Do NOT reinterpret_cast this struct as an array
// as offsets needs to be interpreted
struct EntryColdMeta{
private:
	typedef EntryColdMeta self_type;
	typedef BCF::BCFEntry bcf_type;
	typedef Core::StreamContainer stream_container;
	typedef ColdMetaAllele allele_type;

public:
	explicit EntryColdMeta(void);
	~EntryColdMeta(void);

	// Parse everything in this entry
	bool parse(void);

	// Parse the name only
	bool parseID(void);

	// Parse the alleles only
	bool parseAlleles(void);

	// Write out entry using BCF entry as template
	// and injects into buffer
	bool write(const bcf_type& entry, stream_container& buffer);

public:
	/**< Quality field */
	float QUAL;
	/**< Number of alleles */
	U16 n_allele;
	// Byte length of variant name (ID)
	// Names are limited to 16 bits
	U16 n_ID;
	char* ID;
	// allele info
	// ALTs are limited to 16 bits each
	allele_type* alleles;
};

}
}

#endif /* CORE_BASE_ENTRYCOLDMETA_H_ */
