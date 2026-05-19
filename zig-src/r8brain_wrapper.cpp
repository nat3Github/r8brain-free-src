#define R8B_PFFFT_DOUBLE 1
#include "r8brain_wrapper.h"
#include "../CDSPResampler.h"
#include "../CDSPHBUpsampler.h"
#include "../CDSPHBDownsampler.h"
#include "../CDSPBlockConvolver.h"
#include "../CDSPFracInterpolator.h"
#include "../CDSPFIRFilter.h"

#include <new>
#include <algorithm>

using namespace r8b;

// A simple C++ class to encapsulate the r8brain resampler instance
// and manage its lifetime using RAII.
class R8bResamplerWrapper {
public:
    r8b::CDSPResampler24* resampler;
    // Temporary buffer for converting float input to double for r8brain's process method.
    double* temp_input_buffer;
    int max_input_frames_cached; // Store the max input frames for buffer sizing

    R8bResamplerWrapper(double input_rate, double output_rate, int a_max_in_len, double req_trans_band)
        : resampler(nullptr), temp_input_buffer(nullptr), max_input_frames_cached(a_max_in_len)
    {
        resampler = new r8b::CDSPResampler24(input_rate, output_rate, a_max_in_len, req_trans_band);

        if (resampler != nullptr) {
            temp_input_buffer = new (std::nothrow) double[a_max_in_len];
            if (temp_input_buffer == nullptr) {
                delete resampler;
                resampler = nullptr;
            }
        }
    }

    ~R8bResamplerWrapper() {
        if (resampler) delete resampler;
        if (temp_input_buffer) delete[] temp_input_buffer;
    }
};

// --- C-compatible API implementations ---

R8bResamplerHandle r8b_create_resampler(double input_rate, double output_rate, int a_max_in_len, double req_trans_band) {
    R8bResamplerWrapper* wrapper = new (std::nothrow) R8bResamplerWrapper(input_rate, output_rate, a_max_in_len, req_trans_band);
    if (wrapper == nullptr || wrapper->resampler == nullptr || wrapper->temp_input_buffer == nullptr) {
        if (wrapper != nullptr) delete wrapper;
        return nullptr;
    }
    return wrapper;
}

int r8b_process_resampler(R8bResamplerHandle handle, const float* input, int input_frames, float* output, int output_capacity) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler || !wrapper->temp_input_buffer) return -1;
    if (input_frames > wrapper->max_input_frames_cached) return -2;

    for (int i = 0; i < input_frames; ++i) wrapper->temp_input_buffer[i] = static_cast<double>(input[i]);

    double* r8b_output_ptr = nullptr;
    int produced_frames = wrapper->resampler->process(wrapper->temp_input_buffer, input_frames, r8b_output_ptr);

    if (produced_frames < 0) return -1;

    int frames_to_copy = std::min(produced_frames, output_capacity);
    for (int i = 0; i < frames_to_copy; ++i) output[i] = static_cast<float>(r8b_output_ptr[i]);

    return frames_to_copy;
}

void r8b_destroy_resampler(R8bResamplerHandle handle) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (wrapper) delete wrapper;
}

void r8b_resampler_clear(R8bResamplerHandle handle) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (wrapper && wrapper->resampler) wrapper->resampler->clear();
}

int r8b_resampler_get_latency(R8bResamplerHandle handle) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0;
    return wrapper->resampler->getLatency();
}

double r8b_resampler_get_latency_frac(R8bResamplerHandle handle) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0.0;
    return wrapper->resampler->getLatencyFrac();
}

int r8b_resampler_get_max_out_len(R8bResamplerHandle handle, int max_in_len) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0;
    return wrapper->resampler->getMaxOutLen(max_in_len);
}

int r8b_resampler_get_in_len_before_out_pos(R8bResamplerHandle handle, int req_out_pos) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0;
    return wrapper->resampler->getInLenBeforeOutPos(req_out_pos);
}

int r8b_resampler_get_stage_count(R8bResamplerHandle handle) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0;
    return wrapper->resampler->getStepCount();
}

const char* r8b_resampler_get_stage_name(R8bResamplerHandle handle, int stage_index) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return "";
    if (stage_index < 0 || stage_index >= wrapper->resampler->getStepCount()) return "";
    return wrapper->resampler->getStep(stage_index)->getName();
}

double r8b_resampler_get_stage_latency_frac(R8bResamplerHandle handle, int stage_index) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0.0;
    if (stage_index < 0 || stage_index >= wrapper->resampler->getStepCount()) return 0.0;
    return wrapper->resampler->getStep(stage_index)->getLatencyFrac();
}

int r8b_resampler_get_stage_in_len_before_out_pos(R8bResamplerHandle handle, int stage_index, int req_out_pos) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0;
    if (stage_index < 0 || stage_index >= wrapper->resampler->getStepCount()) return 0;
    return wrapper->resampler->getStep(stage_index)->getInLenBeforeOutPos(req_out_pos);
}

int r8b_resampler_get_stage_kernel_len(R8bResamplerHandle handle, int stage_index) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0;
    if (stage_index < 0 || stage_index >= wrapper->resampler->getStepCount()) return 0;
    return wrapper->resampler->getStep(stage_index)->getKernelLen();
}

int r8b_resampler_get_stage_input_len(R8bResamplerHandle handle, int stage_index) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0;
    if (stage_index < 0 || stage_index >= wrapper->resampler->getStepCount()) return 0;
    return wrapper->resampler->getStep(stage_index)->getInputLen();
}

// --- Component level verification API ---

R8bProcessorHandle r8b_create_hb_upsampler(double req_atten, int steep_index, int is_third, double prev_latency, int do_consume_latency) {
    return new CDSPHBUpsampler(req_atten, steep_index, (is_third != 0), prev_latency, (do_consume_latency != 0));
}

R8bProcessorHandle r8b_create_hb_downsampler(double req_atten, int steep_index, int is_third, double prev_latency) {
    return new CDSPHBDownsampler(req_atten, steep_index, (is_third != 0), prev_latency);
}

R8bProcessorHandle r8b_create_frac_interpolator(double src_rate, double dst_rate, double req_atten, int is_third, double prev_latency) {
    return new CDSPFracInterpolator(src_rate, dst_rate, req_atten, (is_third != 0), prev_latency);
}

R8bFilterHandle r8b_create_lp_filter(double norm_freq, double trans_band, double req_atten, int phase_type, double up_factor) {
    return &CDSPFIRFilterCache::getLPFilter(norm_freq, trans_band, req_atten, (EDSPFilterPhaseResponse)phase_type, up_factor);
}

void r8b_delete_filter(R8bFilterHandle filter) {
    if (filter) static_cast<CDSPFIRFilter*>(filter)->unref();
}

R8bProcessorHandle r8b_create_block_convolver(R8bFilterHandle filter, int up_factor, int down_factor, double prev_latency, int do_consume_latency) {
    return new CDSPBlockConvolver(*static_cast<CDSPFIRFilter*>(filter), up_factor, down_factor, prev_latency, (do_consume_latency != 0));
}

int r8b_process_processor(R8bProcessorHandle handle, const double* input, int input_frames, double* output, int output_capacity) {
    CDSPProcessor* proc = static_cast<CDSPProcessor*>(handle);
    double* out_ptr = output;
    return proc->process(const_cast<double*>(input), input_frames, out_ptr);
}

void r8b_destroy_processor(R8bProcessorHandle handle) {
    if (handle) delete static_cast<CDSPProcessor*>(handle);
}

void r8b_processor_clear(R8bProcessorHandle handle) {
    if (handle) static_cast<CDSPProcessor*>(handle)->clear();
}

int r8b_processor_get_latency(R8bProcessorHandle handle) {
    if (!handle) return 0;
    return static_cast<CDSPProcessor*>(handle)->getLatency();
}

double r8b_processor_get_latency_frac(R8bProcessorHandle handle) {
    if (!handle) return 0.0;
    return static_cast<CDSPProcessor*>(handle)->getLatencyFrac();
}

int r8b_processor_get_max_out_len(R8bProcessorHandle handle, int max_in_len) {
    if (!handle) return 0;
    return static_cast<CDSPProcessor*>(handle)->getMaxOutLen(max_in_len);
}