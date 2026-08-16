#ifndef SINGLETON_H
#define SINGLETON_H

template<typename T>
class Singleton
{
protected:
	Singleton() = default;
	~Singleton() = default;
public:
	static T& Instance(){
		static T instance;
		return instance;
	}
	Singleton(const Singleton&) = delete;
	Singleton& operator =(const Singleton&)=delete;
};

#endif
