#ifndef CORE_GTOBJECT_H_
#define CORE_GTOBJECT_H_

#include "base/MetaEntry.h"

namespace Tachyon{
namespace Core{

struct GTObject{
private:
    typedef GTObject             self_type;
    typedef std::pair<char,char> value_type;
    typedef value_type&          reference;
    typedef const value_type&    const_reference;
    typedef value_type*          pointer;
    typedef const value_type*    const_pointer;
    typedef std::size_t          size_type;

public:
    GTObject(void) : n_alleles(0), n_objects(0), alleles(nullptr){}

    GTObject(const self_type& other) : n_alleles(other.n_alleles), n_objects(other.n_objects), alleles(new value_type[other.n_objects])
    {
    		for(U32 i = 0; i < this->n_alleles; ++i)
    			this->alleles[i] = other.alleles[i];
    }

    GTObject(self_type&& other) noexcept : n_alleles(other.n_alleles), n_objects(other.n_objects), alleles(other.alleles)
    	{
    		other.alleles = nullptr;
    }

    GTObject& operator=(const self_type& other){
    		this->n_alleles = other.n_alleles;
    		if(this->n_alleles == other.n_alleles){
    			for(U32 i = 0; i < this->n_alleles; ++i)
    				this->alleles[i] = other.alleles[i];
    		} else {
    			delete [] this->alleles;
    			this->alleles = new value_type[this->n_alleles];

    			for(U32 i = 0; i < this->n_alleles; ++i)
				this->alleles[i] = other.alleles[i];
    		}

    		this-> n_objects = other.n_objects;
    		return(*this);
    }

    GTObject& operator=(self_type&& other) noexcept{
    		if (this == &other){
			// take precautions against `foo = std::move(foo)`
			return *this;
		}

    		this->n_alleles = other.n_alleles;
    		this->n_objects = other.n_objects;
		delete [] this->alleles;
		this->alleles = other.alleles;
		other.alleles = nullptr;
		return *this;
    }

    virtual ~GTObject(void){ delete [] this->alleles; }

    class iterator{
	private:
		typedef iterator self_type;
		typedef std::forward_iterator_tag iterator_category;

	public:
		iterator(pointer ptr) : ptr_(ptr) { }
		void operator++() { ptr_++; }
		void operator++(int junk) { ptr_++; }
		reference operator*() const{ return *ptr_; }
		pointer operator->() const{ return ptr_; }
		bool operator==(const self_type& rhs) const{ return ptr_ == rhs.ptr_; }
		bool operator!=(const self_type& rhs) const{ return ptr_ != rhs.ptr_; }
	private:
		pointer ptr_;
	};

    class const_iterator{
	private:
		typedef const_iterator self_type;
		typedef std::forward_iterator_tag iterator_category;

	public:
		const_iterator(pointer ptr) : ptr_(ptr) { }
		void operator++() { ptr_++; }
		void operator++(int junk) { ptr_++; }
		const_reference operator*() const{ return *ptr_; }
		const_pointer operator->() const{ return ptr_; }
		bool operator==(const self_type& rhs) const{ return ptr_ == rhs.ptr_; }
		bool operator!=(const self_type& rhs) const{ return ptr_ != rhs.ptr_; }
	private:
		pointer ptr_;
	};

    // Capacity
    inline const size_t size(void) const{ return(this->n_objects); }

    // Element access
    inline pointer         data(void){ return(this->alleles); }
    inline const_pointer   data(void) const{ return(this->alleles); }
    inline reference       operator[](const U32& position){ return(this->alleles[position]); }
    inline const_reference operator[](const U32& position) const{ return(this->alleles[position]); }
    inline reference       at(const U32& position){ return(this->alleles[position]); }
    inline const_reference at(const U32& position) const{ return(this->alleles[position]); }

    // Iterator
    inline iterator       begin(){ return iterator(&this->alleles[0]); }
    inline iterator       end(){ return iterator(&this->alleles[this->n_objects - 1]); }
    inline const_iterator begin() const{ return const_iterator(&this->alleles[0]); }
    inline const_iterator end() const{ return const_iterator(&this->alleles[this->n_objects - 1]); }
    inline const_iterator cbegin() const{ return const_iterator(&this->alleles[0]); }
    inline const_iterator cend() const{ return const_iterator(&this->alleles[this->n_objects - 1]); }

public:
    BYTE      n_alleles;
    size_type n_objects;
    pointer   alleles;
};

struct GTObjectDiploidRLE : public GTObject{
private:
	typedef Core::MetaEntry meta_type;

public:
    GTObjectDiploidRLE(void){}

    template <class T>
    void operator()(const T& gt_primitive, const meta_type& meta_entry)
    {
    		this->__interpret<T>(gt_primitive, meta_entry);
    }

private:
    template <class T>
    void __interpret(const T& gt_primitive, const meta_type& meta_entry)
    {
		this->n_alleles  = 2;
		this->alleles    = new std::pair<char,char>[2];
		const BYTE shift = meta_entry.isAnyGTMissing()   ? 2 : 1;
		const BYTE add   = meta_entry.isGTMixedPhasing() ? 1 : 0;

		if(add) this->alleles[0].second = gt_primitive & 1;
		else    this->alleles[0].second = meta_entry.getControllerPhase();

		this->alleles[0].first = (gt_primitive & ((1 << shift) - 1) << add) >> add;
		this->alleles[1].first = (gt_primitive & ((1 << shift) - 1) << (add+shift)) >> (add+shift);
		this->n_objects        = gt_primitive >> (2*shift + add);
	}
};

struct GTObjectDiploidSimple : public GTObject{
private:
	typedef Core::MetaEntry meta_type;

public:
	GTObjectDiploidSimple(void){}

    template <class T>
    void operator()(const T& gt_primitive, const meta_type& meta_entry)
    {
    		this->__interpret<T>(gt_primitive, meta_entry);
    }

private:
    template <class T>
    void __interpret(const T& gt_primitive, const meta_type& meta_entry)
    {
		this->n_alleles     = 2;
		this->alleles       = new std::pair<char,char>[2];
		const BYTE shift    = ceil(log2(meta_entry.getNumberAlleles() + 1 + meta_entry.isAnyGTMissing())); // Bits occupied per allele, 1 value for missing
		const BYTE add      = meta_entry.isGTMixedPhasing() ? 1 : 0;

		if(add) this->alleles[0].second = gt_primitive & 1;
		else    this->alleles[0].second = meta_entry.getControllerPhase();

		this->alleles[0].first = (gt_primitive & ((1 << shift) - 1) << add) >> add;
		this->alleles[1].first = (gt_primitive & ((1 << shift) - 1) << (add+shift)) >> (add+shift);
		this->n_objects        = gt_primitive >> (2*shift + add);
	}
};

}
}



#endif /* GTOBJECT_H_ */