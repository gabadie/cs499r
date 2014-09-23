
#include "cs499rPrefix.hpp"


class DummyClass
{
public:
    int a;


    DummyClass()
    {
        a = 0;
    }

    void b()
    {
        a = 2;
    }

};

void hello()
{
    DummyClass a;

    a.b();
}
