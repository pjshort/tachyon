#ifndef CORE_VARIANT_IMPORTER_CONTAINER_STATS_H_
#define CORE_VARIANT_IMPORTER_CONTAINER_STATS_H_

#include "../containers/datacontainer.h"

namespace tachyon{
namespace support{

struct VariantImporterStatsObject{
	typedef VariantImporterStatsObject self_type;
	typedef containers::DataContainer data_container_type;

	VariantImporterStatsObject(void) : cost_uncompressed(0), cost_compressed(0){}
	~VariantImporterStatsObject(){}

	void operator+=(const data_container_type& container){
		this->cost_uncompressed += container.getObjectSizeUncompressed();
		this->cost_compressed   += container.getObjectSize();
	}

	friend std::ostream& operator<<(std::ostream& stream, const self_type& entry){
		stream << entry.cost_compressed << '\t' << entry.cost_uncompressed << '\t' <<
				(entry.cost_compressed ? (double)entry.cost_uncompressed/entry.cost_compressed : 0);
		return(stream);
	}

	U64 cost_uncompressed;
	U64 cost_compressed;
};

class VariantImporterContainerStats{
private:
	typedef VariantImporterContainerStats self_type;
	typedef VariantImporterStatsObject    value_type;
    typedef std::size_t                   size_type;
    typedef value_type&                   reference;
    typedef const value_type&             const_reference;
    typedef value_type*                   pointer;
    typedef const value_type*             const_pointer;

public:
    VariantImporterContainerStats() :
    	n_entries_(0),
		n_capacity_(200),
		entries_(static_cast<pointer>(::operator new[](this->capacity()*sizeof(value_type))))
    {
    	for(U32 i = 0; i < this->capacity(); ++i)
    		new( &this->entries_[i] ) value_type( );
    }

    VariantImporterContainerStats(const size_type start_capacity) :
    	n_entries_(0),
		n_capacity_(start_capacity),
		entries_(static_cast<pointer>(::operator new[](this->capacity()*sizeof(value_type))))
    {
    	for(U32 i = 0; i < this->capacity(); ++i)
			new( &this->entries_[i] ) value_type( );
    }

    ~VariantImporterContainerStats(){
    	for(std::size_t i = 0; i < this->size(); ++i)
			(this->entries_ + i)->~VariantImporterStatsObject();

		::operator delete[](static_cast<void*>(this->entries_));
    }

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

	// Element access
	inline reference at(const size_type& position){ return(this->entries_[position]); }
	inline const_reference at(const size_type& position) const{ return(this->entries_[position]); }
	inline reference operator[](const size_type& position){ return(this->entries_[position]); }
	inline const_reference operator[](const size_type& position) const{ return(this->entries_[position]); }
	inline pointer data(void){ return(this->entries_); }
	inline const_pointer data(void) const{ return(this->entries_); }
	inline reference front(void){ return(this->entries_[0]); }
	inline const_reference front(void) const{ return(this->entries_[0]); }
	inline reference back(void){ return(this->entries_[this->n_entries_ - 1]); }
	inline const_reference back(void) const{ return(this->entries_[this->n_entries_ - 1]); }

	// Capacity
	inline const bool empty(void) const{ return(this->n_entries_ == 0); }
	inline const size_type& size(void) const{ return(this->n_entries_); }
	inline const size_type& capacity(void) const{ return(this->n_capacity_); }

	// Iterator
	inline iterator begin(){ return iterator(&this->entries_[0]); }
	inline iterator end(){ return iterator(&this->entries_[this->n_entries_]); }
	inline const_iterator begin() const{ return const_iterator(&this->entries_[0]); }
	inline const_iterator end() const{ return const_iterator(&this->entries_[this->n_entries_]); }
	inline const_iterator cbegin() const{ return const_iterator(&this->entries_[0]); }
	inline const_iterator cend() const{ return const_iterator(&this->entries_[this->n_entries_]); }

	//
	inline self_type& operator+=(const const_reference entry){
		if(this->size() + 1 == this->n_capacity_)
			this->resize();


		this->entries_[this->n_entries_++] = entry;
		return(*this);
	}
	inline self_type& add(const const_reference entry){ return(*this += entry); }

	void resize(void){
		pointer temp = this->entries_;

		this->n_capacity_ *= 2;
		this->entries_ = static_cast<pointer>(::operator new[](this->capacity()*sizeof(value_type)));

		// Lift over values from old addresses
		for(U32 i = 0; i < this->size(); ++i)
			new( &this->entries_[i] ) value_type( temp[i] );


		for(std::size_t i = 0; i < this->size(); ++i)
			(temp + i)->~VariantImporterStatsObject();

		::operator delete[](static_cast<void*>(temp));
	}

private:
	size_type n_entries_;
	size_type n_capacity_;
	pointer   entries_;
};

}
}

#endif /* CORE_VARIANT_IMPORTER_CONTAINER_STATS_H_ */
