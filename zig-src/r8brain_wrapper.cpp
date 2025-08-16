#define R8B_PFFFT_DOUBLE 1
#include "r8brain_wrapper.h"
#include "../CDSPResampler.h" // Assuming this header defines CDSPResampler24
#include <new> // For std::nothrow
#include <algorithm> // For std::min

// A simple C++ class to encapsulate the r8brain resampler instance
// and manage its lifetime using RAII.
class R8bResamplerWrapper {
public:
    r8b::CDSPResampler24* resampler;
    // Temporary buffer for converting float input to double for r8brain's process method.
    double* temp_input_buffer;
    int max_input_frames_cached; // Store the max input frames for buffer sizing

    // Constructor: Creates the r8brain resampler and its internal temp buffer.
    R8bResamplerWrapper(double input_rate, double output_rate, int a_max_in_len, double req_trans_band)
        : resampler(nullptr), temp_input_buffer(nullptr), max_input_frames_cached(a_max_in_len)
    {
        // Allocate the r8brain resampler.
        // If this allocation fails and you're compiling with `-fno-exceptions`,
        // the program will typically terminate.
        resampler = new r8b::CDSPResampler24(input_rate, output_rate, a_max_in_len, req_trans_band);

        // Allocate the temporary input buffer for float-to-double conversion.
        if (resampler != nullptr) {
            temp_input_buffer = new (std::nothrow) double[a_max_in_len];
            if (temp_input_buffer == nullptr) {
                delete resampler;
                resampler = nullptr;
            }
        }
    }

    // Destructor: Deletes the r8brain resampler and temporary buffer.
    ~R8bResamplerWrapper() {
        if (resampler) {
            delete resampler;
        }
        if (temp_input_buffer) {
            delete[] temp_input_buffer;
        }
    }
};

// --- C-compatible API implementations ---

R8bResamplerHandle r8b_create_resampler(double input_rate, double output_rate, int a_max_in_len, double req_trans_band) {
    R8bResamplerWrapper* wrapper = new (std::nothrow) R8bResamplerWrapper(input_rate, output_rate, a_max_in_len, req_trans_band);

    if (wrapper == nullptr || wrapper->resampler == nullptr || wrapper->temp_input_buffer == nullptr) {
        if (wrapper != nullptr) {
            delete wrapper;
        }
        return nullptr; // Indicate failure
    }

    return wrapper;
}

int r8b_process_resampler(R8bResamplerHandle handle, const float* input, int input_frames, float* output, int output_capacity) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler || !wrapper->temp_input_buffer) {
        return -1; // Invalid handle or resampler/buffers not initialized
    }

    // Ensure `input_frames` does not exceed the `max_input_frames_cached`
    if (input_frames > wrapper->max_input_frames_cached) {
        return -2; // Error: too many input frames
    }

    // Convert `float` input to `double` for r8brain's `process` method.
    for (int i = 0; i < input_frames; ++i) {
        wrapper->temp_input_buffer[i] = static_cast<double>(input[i]);
    }

    double* r8b_output_ptr = nullptr;
    int produced_frames = wrapper->resampler->process(wrapper->temp_input_buffer, input_frames, r8b_output_ptr);

    if (produced_frames < 0) {
        return -1; // r8brain indicated an internal error
    }

    // Copy produced frames from r8brain's internal buffer to the user-provided `float` output buffer.
    int frames_to_copy = std::min(produced_frames, output_capacity);
    for (int i = 0; i < frames_to_copy; ++i) {
        output[i] = static_cast<float>(r8b_output_ptr[i]);
    }

    return frames_to_copy;
}

void r8b_destroy_resampler(R8bResamplerHandle handle) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (wrapper) {
        delete wrapper;
    }
}

void r8b_resampler_clear(R8bResamplerHandle handle) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (wrapper && wrapper->resampler) {
        wrapper->resampler->clear();
    }
}

int r8b_resampler_get_in_len_before_out_pos(R8bResamplerHandle handle, int req_out_pos) {
    R8bResamplerWrapper* wrapper = static_cast<R8bResamplerWrapper*>(handle);
    if (!wrapper || !wrapper->resampler) return 0;
    return wrapper->resampler->getInLenBeforeOutPos(req_out_pos);
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