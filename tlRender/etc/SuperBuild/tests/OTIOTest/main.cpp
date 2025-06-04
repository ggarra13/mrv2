#include <opentime/rationalTime.h>

#include <iostream>

int main(int argc, char* argv[])
{
    opentime::OPENTIME_VERSION::RationalTime t(1.0, 24.0);
    std::cout << "Rational time: " << t.value() << "/" << t.rate() << std::endl;
    return 0;
}
