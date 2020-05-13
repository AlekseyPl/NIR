#ifndef Math_Fftwf_H_
#define Math_Fftwf_H_


#include <Math/Complex.h>
#include <fftw3.h>

namespace Math {

class Fftwf {
public:
    enum Direction {
        dirForward = FFTW_FORWARD, dirBackward = FFTW_BACKWARD
    };

    struct Params {
        Params(ComplexFloat* in, ComplexFloat* out, uint32_t size, Direction direction = dirForward) :
                in(in), out(out), size(size), direction(direction) {
        }
        ComplexFloat* in;
        ComplexFloat* out;
        uint32_t size;
        Direction direction;
    };

    Fftwf(const Params &params);
    virtual ~Fftwf();

    int32_t Do();
    static int32_t Do(const Params &params);

private:
    fftwf_plan plan;
};

}

#endif //Math_Fftwf_H_
