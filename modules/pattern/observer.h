#include <string>

template <typename T> // added
class IObserver {
    public:
        virtual ~IObserver(){};
        virtual void Update(*T) = 0;
};

template <typename T> // added
class ISubject {
    public:
        virtual ~ISubject(){};
        virtual void Attach(IObserver<T> *observer) = 0;
        virtual void Detach(IObserver<T> *observer) = 0;
        virtual void Notify() = 0;
};
