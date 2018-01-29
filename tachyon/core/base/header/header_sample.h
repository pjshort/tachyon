#ifndef CORE_BASE_HEADERSAMPLE_H_
#define CORE_BASE_HEADERSAMPLE_H_

namespace tachyon{
namespace core{

struct HeaderSample{
private:
	typedef HeaderSample self_type;

public:
	HeaderSample(void){}
	HeaderSample(const std::string& name) : name(name){}
	~HeaderSample(){}

	// Capacity
	inline const bool empty(void) const{ return(this->name.size() == 0); }
	inline const size_t size(void) const{ return(this->name.size()); }

	// Element access
	inline char* data(void){ return(&this->name[0]); }
	inline const char* const data(void) const{ return(&this->name[0]); }

private:
	friend std::ofstream& operator<<(std::ofstream& stream, const self_type& entry){
		const U32 l_name = entry.name.size();
		stream.write(reinterpret_cast<const char*>(&l_name), sizeof(U32));
		stream.write(&entry.name[0], entry.name.size());
		return(stream);
	}

	friend std::ifstream& operator>>(std::ifstream& stream, self_type& entry){
		U32 l_name = 0;
		stream.read(reinterpret_cast<char*>(&l_name), sizeof(U32));
		entry.name.resize(l_name);
		stream.read(&entry.name[0], l_name);
		return(stream);
	}

public:
	std::string name;
};

}
}



#endif /* CORE_BASE_HEADERSAMPLE_H_ */