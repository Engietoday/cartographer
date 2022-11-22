#ifndef MODULE_H
#define MODULE_H

class Module{
    public:
        virtual void run_10ms(void) = 0;
        virtual void run_20ms(void) = 0;
        virtual void run_50ms(void) = 0;
        virtual void run_100ms(void) = 0;
        virtual void run_1000ms(void) = 0;
};



#endif