#ifndef CONTAINER_PRIMITIVECONTAINER_H_
#define CONTAINER_PRIMITIVECONTAINER_H_

#include <typeinfo>

#include "../math/summary_statistics.h"
#include "variantblock.h"

namespace tachyon{
namespace containers{

template <class return_type>
class PrimitiveContainer{
private:
    typedef std::size_t       size_type;
    typedef return_type       value_type;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef std::ptrdiff_t    difference_type;
    typedef DataContainer     container_type;

public:
    PrimitiveContainer();
    PrimitiveContainer(const container_type& container);
    PrimitiveContainer(const container_type& container, const U32& offset, const U32 n_entries);
    ~PrimitiveContainer(void);

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
    inline reference at(const size_type& position){ return(this->__entries[position]); }
    inline const_reference at(const size_type& position) const{ return(this->__entries[position]); }
    inline reference operator[](const size_type& position){ return(this->__entries[position]); }
    inline const_reference operator[](const size_type& position) const{ return(this->__entries[position]); }
    inline pointer data(void){ return(this->__entries); }
    inline const_pointer data(void) const{ return(this->__entries); }
    inline reference front(void){ return(this->__entries[0]); }
    inline const_reference front(void) const{ return(this->__entries[0]); }
    inline reference back(void){ return(this->__entries[this->n_entries - 1]); }
    inline const_reference back(void) const{ return(this->__entries[this->n_entries - 1]); }

    // Capacity
    inline const bool empty(void) const{ return(this->n_entries == 0); }
    inline const size_type& size(void) const{ return(this->n_entries); }
    inline const bool isUniform(void) const{ return(this->__uniform); }

    // Iterator
    inline iterator begin(){ return iterator(&this->__entries[0]); }
    inline iterator end(){ return iterator(&this->__entries[this->n_entries]); }
    inline const_iterator begin() const{ return const_iterator(&this->__entries[0]); }
    inline const_iterator end() const{ return const_iterator(&this->__entries[this->n_entries]); }
    inline const_iterator cbegin() const{ return const_iterator(&this->__entries[0]); }
    inline const_iterator cend() const{ return const_iterator(&this->__entries[this->n_entries]); }

private:
    template <class native_primitive>
    void __setup(const container_type& container, const U32& offset);

    template <class native_primitive>
    void __setupSigned(const container_type& container, const U32& offset);

private:
    bool    __uniform;
    size_t  n_entries;
    pointer __entries;
};


// IMPLEMENTATION -------------------------------------------------------------


template <class return_type>
PrimitiveContainer<return_type>::PrimitiveContainer() :
	__uniform(false),
	n_entries(0),
	__entries(nullptr)
{

}

template <class return_type>
PrimitiveContainer<return_type>::PrimitiveContainer(const container_type& container) :
	__uniform(false),
	n_entries(0),
	__entries(nullptr)
{
	if(container.header.data_header.getPrimitiveWidth() == -1)
		return;

	assert(container.buffer_data_uncompressed.size() % container.header.data_header.getPrimitiveWidth() == 0);

	this->n_entries = container.buffer_data_uncompressed.size() / container.header.data_header.getPrimitiveWidth();
	this->__entries = new value_type[this->n_entries];

	if(this->n_entries == 0)
		return;

	this->__uniform = container.header.data_header.isUniform();

	if(container.header.data_header.isSigned()){
		switch(container.header.data_header.getPrimitiveType()){
		case(YON_TYPE_8B):     (this->__setupSigned<SBYTE>(container, 0));  break;
		case(YON_TYPE_16B):    (this->__setupSigned<S16>(container, 0));    break;
		case(YON_TYPE_32B):    (this->__setupSigned<S32>(container, 0));    break;
		case(YON_TYPE_64B):    (this->__setupSigned<S64>(container, 0));    break;
		case(YON_TYPE_FLOAT):  (this->__setup<float>(container, 0));        break;
		case(YON_TYPE_DOUBLE): (this->__setup<double>(container, 0));       break;
		default: std::cerr << "Disallowed: " << container.header.data_header.getPrimitiveType() << std::endl; return;
		}
	} else {
		switch(container.header.data_header.getPrimitiveType()){
		case(YON_TYPE_8B):     (this->__setup<BYTE>(container, 0));   break;
		case(YON_TYPE_16B):    (this->__setup<U16>(container, 0));    break;
		case(YON_TYPE_32B):    (this->__setup<U32>(container, 0));    break;
		case(YON_TYPE_64B):    (this->__setup<U64>(container, 0));    break;
		case(YON_TYPE_FLOAT):  (this->__setup<float>(container, 0));  break;
		case(YON_TYPE_DOUBLE): (this->__setup<double>(container, 0)); break;
		default: std::cerr << "Disallowed: " << container.header.data_header.getPrimitiveType() << std::endl; return;
		}
	}
}

template <class return_type>
PrimitiveContainer<return_type>::PrimitiveContainer(const container_type& container,
                                                              const U32& offset,
                                                              const U32  n_entries) :
	__uniform(false),
	n_entries(n_entries),
	__entries(new value_type[n_entries])
{
	if(container.header.data_header.isSigned()){
		switch(container.header.data_header.getPrimitiveType()){
		case(YON_TYPE_8B):     (this->__setupSigned<SBYTE>(container, offset));  break;
		case(YON_TYPE_16B):    (this->__setupSigned<S16>(container, offset));    break;
		case(YON_TYPE_32B):    (this->__setupSigned<S32>(container, offset));    break;
		case(YON_TYPE_64B):    (this->__setupSigned<S64>(container, offset));    break;
		case(YON_TYPE_FLOAT):  (this->__setup<float>(container, offset));        break;
		case(YON_TYPE_DOUBLE): (this->__setup<double>(container, offset));       break;
		default: std::cerr << "Disallowed" << std::endl; return;
		}
	} else {
		switch(container.header.data_header.getPrimitiveType()){
		case(YON_TYPE_8B):     (this->__setup<BYTE>(container, offset));   break;
		case(YON_TYPE_16B):    (this->__setup<U16>(container, offset));    break;
		case(YON_TYPE_32B):    (this->__setup<U32>(container, offset));    break;
		case(YON_TYPE_64B):    (this->__setup<U64>(container, offset));    break;
		case(YON_TYPE_FLOAT):  (this->__setup<float>(container, offset));  break;
		case(YON_TYPE_DOUBLE): (this->__setup<double>(container, offset)); break;
		default: std::cerr << "Disallowed" << std::endl; return;
		}
	}
}

template <class return_type>
PrimitiveContainer<return_type>::~PrimitiveContainer(void){
	delete [] this->__entries;
}

template <class return_type>
template <class native_primitive>
void PrimitiveContainer<return_type>::__setup(const container_type& container, const U32& offset){
	const native_primitive* const data = reinterpret_cast<const native_primitive* const>(&container.buffer_data_uncompressed.buffer[offset]);

	for(U32 i = 0; i < this->size(); ++i)
		this->__entries[i] = data[i];
}

template <class return_type>
template <class native_primitive>
void PrimitiveContainer<return_type>::__setupSigned(const container_type& container, const U32& offset){
	const native_primitive* const data = reinterpret_cast<const native_primitive* const>(&container.buffer_data_uncompressed.buffer[offset]);

	if(sizeof(native_primitive) == sizeof(return_type)){
		return(this->__setup<native_primitive>(container, offset));
	}
	else {
		for(U32 i = 0; i < this->size(); ++i){
			// If the data is missing in the native format
			if(data[i] == std::numeric_limits<native_primitive>::min()){
				this->__entries[i] = std::numeric_limits<return_type>::min();
			}
			// If the data is EOV in the native format
			else if(data[i] == std::numeric_limits<native_primitive>::min() + 1){
				//std::cerr << "is eov: " << std::bitset<sizeof(native_primitive)*8>(data[i]) << "\t" << std::bitset<sizeof(return_type)*8>(std::numeric_limits<return_type>::min() + 1) << std::endl;
				this->__entries[i] = std::numeric_limits<return_type>::min() + 1;
			}
			else
				this->__entries[i] = data[i];
		}
	}
}

}
}



#endif /* PrimitiveContainer_H_ */
