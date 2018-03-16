#include "meta_entry.h"

namespace tachyon{
namespace core{

MetaEntry::MetaEntry() :
	n_alleles(0),
	info_pattern_id(-1),
	filter_pattern_id(-1),
	format_pattern_id(-1),
	quality(NAN),
	contigID(0),
	position(0),
	alleles(nullptr)
{}

MetaEntry::~MetaEntry(){
	//delete [] this->alleles;
	for(std::size_t i = 0; i < this->n_alleles; ++i)
		(this->alleles + i)->~MetaAllele();

	::operator delete[](static_cast<void*>(this->alleles));
};

void MetaEntry::toVCFString(std::ostream& dest, const header_type& header) const{
	exit(1);
	/*
	dest.write(&header.getContig(this->hot.contigID).name[0], header.getContig(this->hot.contigID).name.size()) << '\t';
	dest << this->hot.position + 1 << '\t';

	// If we have cold meta
	if(this->loaded_cold){
		if(this->cold.n_ID == 0) dest.put('.');
		else dest.write(this->cold.ID, this->cold.n_ID);
		dest << '\t';
		if(this->hot.controller.biallelic && this->hot.controller.simple_snv){
			dest << this->hot.ref_alt.getRef() << '\t' << this->hot.ref_alt.getAlt();
		}
		else {
			dest.write(this->cold.alleles[0].allele, this->cold.alleles[0].l_allele);
			dest << '\t';
			U16 j = 1;
			for(; j < this->cold.n_allele - 1; ++j){
				dest.write(this->cold.alleles[j].allele, this->cold.alleles[j].l_allele);
				dest.put(',');
			}
			dest.write(this->cold.alleles[j].allele, this->cold.alleles[j].l_allele);
		}
		if(std::isnan(this->cold.QUAL)) dest << "\t.\t";
		else {
			dest << '\t' << this->cold.QUAL << '\t';
		}
	}
	*/
}

void MetaEntry::toVCFString(buffer_type& dest, const header_type& header) const{
	exit(1);

	/*
	dest.Add(&header.getContig(this->hot.contigID).name[0], header.getContig(this->hot.contigID).name.size());
	dest += '\t';
	//dest += std::to_string(blockPos + this->hot.position + 1);

	if(dest.size() + 100 >= dest.capacity()){
		//std::cerr << "resizing: " << dest.capacity() << "->" << dest.capacity()*2 << std::endl;
		dest.resize(dest.capacity()*2);
	}
	assert(dest.size() + 100 < dest.capacity());
	int ret = sprintf(&dest.buffer[dest.size()], "%llu", this->hot.position + 1);
	dest.n_chars += ret;
	dest += '\t';

	// If we have cold meta
	if(this->loaded_cold){
		if(this->cold.n_ID == 0) dest += '.';
		else dest.Add(this->cold.ID, this->cold.n_ID);
		dest += '\t';
		if(this->hot.controller.biallelic && this->hot.controller.simple_snv){
			dest += this->hot.ref_alt.getRef();
			dest += '\t';
			dest += this->hot.ref_alt.getAlt();
		}
		else {
			dest.Add(this->cold.alleles[0].allele, this->cold.alleles[0].l_allele);
			dest += '\t';
			U16 j = 1;
			for(; j < this->cold.n_allele - 1; ++j){
				dest.Add(this->cold.alleles[j].allele, this->cold.alleles[j].l_allele);
				dest += ',';
			}
			dest.Add(this->cold.alleles[j].allele, this->cold.alleles[j].l_allele);
		}
		if(std::isnan(this->cold.QUAL)) dest += "\t.\t";
		else {
			dest += '\t';
			ret = sprintf(&dest.buffer[dest.size()], "%g", this->cold.QUAL);
			dest.n_chars += ret;
			dest += '\t';
		}
	}
	*/
}

}
}
