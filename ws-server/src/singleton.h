/*================================================================
*   Copyright (c) 2021, Inc. All Rights Reserved.
*   
*   @file：singleton.h
*   @author：linju
*   @email：15013144713@163.com
*   @date ：2021-12-11
*   @berief： Singleton template
*
================================================================*/
#ifndef _BASE_SINGLETON_TEMPLATE_H_
#define _BASE_SINGLETON_TEMPLATE_H_

#include <atomic>
#include <mutex>

template <typename T> 
class Singleton {
public:
    template<typename... Args>
    static T& ArgsInstance(Args&&... args) {
        if (nullptr == value_) {
            value_ = new T(std::forward<Args>(args)...);
        }
        return *value_;
    }
    
    // TODO: Double check lock mode
    static T& Instance() {
        if (nullptr == value_) {
            value_ = new T();
        }
        return *value_;
    }
private:
    Singleton() {}
    ~Singleton() = default;

    Singleton(const Singleton& rhs) = delete;
    Singleton operator=(const Singleton& rhs) = delete;

    static void destroy()
	{
		delete value_;
	}

private:
    static T* value_;
    std::mutex mutex_;
};

template<typename T>
T* Singleton<T>::value_ = nullptr;


#endif // _BASE_SINGLETON_TEMPLATE_H_