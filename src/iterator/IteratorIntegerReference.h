#ifndef ITERATOR_ITERATORINTEGERREFERENCE_H_
#define ITERATOR_ITERATORINTEGERREFERENCE_H_

#include "../io/BasicBuffer.h"

namespace Tachyon{
namespace Iterator{

template <class return_primitive_type = U32>
class IteratorIntegerReference{
    typedef IteratorIntegerReference self_type;
    typedef IO::BasicBuffer          buffer_type;
    typedef return_primitive_type    T;

public:
    IteratorIntegerReference(char* const data);
    IteratorIntegerReference(const self_type& other);
    IteratorIntegerReference(self_type&& other) noexcept;
    IteratorIntegerReference& operator=(const self_type& other);
    IteratorIntegerReference& operator=(self_type&& other) noexcept;
    virtual ~IteratorIntegerReference();

    // Iterators
    inline void advance(const U32& steps){ (*this) += steps; }
    inline void retreat(const U32& steps){ (*this) -= steps; }
    //virtual void begin() =0;
    //virtual void end()   =0;
    //virtual void prev()  =0;
    //virtual void next()  =0;
    //virtual void first() =0;

    // Iterator control
    inline void reset(void){ this->current_position = 0; }

    // Element access
    virtual T operator[](const U32& position) const =0;
    virtual T operator*(void) const =0;

    inline void operator++(void){
        if(this->current_position + 1 == this->n_entries) return;
        ++this->current_position;
    }

    inline void operator+=(const size_t& steps){
        if(this->current_position + steps >= this->n_entries) return;
        this->current_position += steps;
    }

    inline void operator--(void){
        if(this->current_position == 0) return;
        --this->current_position;
    }

    inline void operator-=(const size_t& steps){
        if(this->current_position - steps < 0) return;
        this->current_position -= steps;
    }

    virtual T at(const U32& position) const =0;
    virtual T front(void) const =0;
    virtual T back(void) const =0;

    // Capacity
    inline const size_t& size(void) const{ return(this->n_entries); }
    inline const bool empty(void) const{ return(this->n_entries == 0); }

    // Basic mathematical functions
    virtual float mathMean(void) const =0;
    virtual float mathAverage(void) const =0;
    virtual U64 mathMax(void) const =0;
    virtual U64 mathMin(void) const =0;

    // Convertions to string
    virtual void toString(std::ostream& stream = std::cout) =0;
    virtual void toString(buffer_type& buffer) =0;

private:
    friend std::ostream& operator<<(std::ostream& os, const self_type& iterator);

protected:
    size_t current_position;
    size_t n_entries;
    char* const data;
};

template <class return_primitive_type, class parent_primitive_type = U32>
class IteratorIntegerReferenceImpl : public IteratorIntegerReference<parent_primitive_type>{
private:
    typedef IteratorIntegerReferenceImpl self_type;
    typedef IteratorIntegerReference<parent_primitive_type> parent_type;
    typedef IO::BasicBuffer          buffer_type;
    typedef return_primitive_type        T;

public:
    IteratorIntegerReferenceImpl(const char* const data);
    ~IteratorIntegerReferenceImpl();

    // Element access
    inline T operator[](const U32& position) const{ return(this->__data_interpreted[position]); }
    inline T operator*() const{ return(this->__data_interpreted[this->current_position]); }
    inline T at(const U32& position) const{ return(this->__data_interpreted[position]); }
    inline T front(void) const{ return(this->__data_interpreted[0]); }
    inline T back(void) const{
        if(this->n_entries == 0)
            return(this->front());
        return(this->__data_interpreted[this->n_entries - 1]);
    }

    // Basic mathematical functions
    float mathMean(void) const{
        if(this->n_entries == 0) return 0;

        float total = 0;
        for(U32 i = 0; i < this->n_entries; ++i)
            total += this->at(i);

        return(total / this->n_entries);
    }

    inline float mathAverage(void) const{ return(this->mathMean()); }

    U64 mathMax(void) const{
        if(this->n_entries == 0) return 0;
        U64 current_max  = 0;

        for(U32 i = 0; i < this->n_entries; ++i)
            if(this->at(i) > current_max) current_max = this->at(i);

        return(current_max);
    }

    U64 mathMin(void) const{
        if(this->n_entries == 0) return 0;
        U64 current_min = std::numeric_limits<U64>::max();

        for(U32 i = 0; i < this->n_entries; ++i)
            if(this->at(i) < current_min) current_min = this->at(i);

        return(current_min);
    }

    // Convertions to string
    void toString(std::ostream& stream = std::cout);
    void toString(buffer_type& buffer);

private:
    T* const __data_interpreted;
};

template <class return_primitive_type>
IteratorIntegerReference<return_primitive_type>::IteratorIntegerReference(char* const data) :
	current_position(0),
	n_entries(0),
	data(data)
{

}

template <class return_primitive_type>
IteratorIntegerReference<return_primitive_type>::~IteratorIntegerReference(){}

template <class return_primitive_type, class parent_primitive_type>
IteratorIntegerReferenceImpl<return_primitive_type, parent_primitive_type>::IteratorIntegerReferenceImpl(const char* const data) :
	parent_type(data),
	__data_interpreted(data)
{
}

template <class return_primitive_type, class parent_primitive_type>
IteratorIntegerReferenceImpl<return_primitive_type, parent_primitive_type>::~IteratorIntegerReferenceImpl(){}


}
}



#endif /* ITERATOR_ITERATORINTEGERREFERENCE_H_ */
