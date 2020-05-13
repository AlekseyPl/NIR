#include "Fftwf.h"

namespace Math {

Fftwf::Fftwf(const Params &params) {
    plan = fftwf_plan_dft_1d(params.size, reinterpret_cast<fftwf_complex*>(params.in),
            reinterpret_cast<fftwf_complex*>(params.out), params.direction, FFTW_ESTIMATE);
}

Fftwf::~Fftwf() {
    fftwf_destroy_plan(plan);
    fftwf_cleanup();
}

int32_t Fftwf::Do() {
    fftwf_execute(plan);
    return 0;
}

int32_t Fftwf::Do(const Params &params) {
    fftwf_plan plan = fftwf_plan_dft_1d(params.size, reinterpret_cast<fftwf_complex*>(params.in),
            reinterpret_cast<fftwf_complex*>(params.out), params.direction, FFTW_ESTIMATE);
    fftwf_execute(plan);
    fftwf_destroy_plan(plan);
    fftwf_cleanup();
    return 0;
}

} /* namespace Umts */
