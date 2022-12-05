#ifndef OBSERVER_H
#define OBSERVER_H

#include <string>

template <class T> // added
class IObserver {
    public:
        virtual ~IObserver(){};
        virtual void Update(T) = 0;
};

template <class T> // added
class ISubject {
    public:
        virtual ~ISubject(){};
        virtual void Attach(IObserver<T> *observer) = 0;
        virtual void Detach(IObserver<T> *observer) = 0;
        virtual void Notify() = 0;
};

#endif