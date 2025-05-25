#ifndef R8BRAIN_WRAPPER_H
#define R8BRAIN_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// Opaque pointer for the resampler instance.
typedef void* R8bResamplerHandle;

/**
 * @brief Creates a new r8brain resampler instance.
 * @param input_rate The input sample rate.
 * @param output_rate The output sample rate.
 * @param a_max_in_len The maximum number of input frames
 * that will be processed in a single call to `r8b_process_resampler`.
 * @param req_trans_band The required transition band. Default is 2.0.
 * @return An opaque handle to the resampler instance, or NULL on failure.
 */
R8bResamplerHandle r8b_create_resampler(double input_rate, double output_rate, int a_max_in_len, double req_trans_band);

/**
 * @brief Processes audio samples using the resampler.
 * @param handle The resampler instance handle.
 * @param input A pointer to the input audio data (mono float samples).
 * @param input_frames The number of input frames available.
 * @param output A pointer to the buffer where output audio data will be written.
 * @param output_capacity The maximum number of output frames that can be written
 * to the `output` buffer.
 * @return The number of output frames actually produced, or -1 on error.
 */
int r8b_process_resampler(R8bResamplerHandle handle, const float* input, int input_frames, float* output, int output_capacity);

/**
 * @brief Destroys an r8brain resampler instance and frees its resources.
 * @param handle The resampler instance handle.
 */
void r8b_destroy_resampler(R8bResamplerHandle handle);

/**
 * @brief Clears the internal buffers and resets the state of the resampler.
 * @param handle The resampler instance handle.
 */
void r8b_resampler_clear(R8bResamplerHandle handle);

/**
 * @brief Returns the number of input frames before a specified output position.
 * @param handle The resampler instance handle.
 * @param req_out_pos The requested output position.
 * @return The number of input frames, or 0 if handle is invalid.
 */
int r8b_resampler_get_in_len_before_out_pos(R8bResamplerHandle handle, int req_out_pos);

/**
 * @brief Gets the total latency of the resampler in samples (integer part).
 * @param handle The resampler instance handle.
 * @return The latency in samples, or 0 if handle is invalid.
 */
int r8b_resampler_get_latency(R8bResamplerHandle handle);

/**
 * @brief Gets the fractional part of the resampler's latency.
 * @param handle The resampler instance handle.
 * @return The fractional latency value, or 0.0 if handle is invalid.
 */
double r8b_resampler_get_latency_frac(R8bResamplerHandle handle);

/**
 * @brief Returns the maximum possible number of output frames that can be generated
 * for a given maximum number of input frames.
 * @param handle The resampler instance handle.
 * @param max_in_len The maximum number of input frames.
 * @return Max output frames, or 0 if handle is invalid.
 */
int r8b_resampler_get_max_out_len(R8bResamplerHandle handle, int max_in_len);

#ifdef __cplusplus
}
#endif

#endif // R8BRAIN_WRAPPER_H