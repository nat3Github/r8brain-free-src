#ifndef R8BRAIN_WRAPPER_H
#define R8BRAIN_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// Opaque pointer for the resampler instance.
typedef void* R8bResamplerHandle;
typedef void* R8bProcessorHandle;
typedef void* R8bFilterHandle;

/**
 * @brief Creates a new r8brain resampler instance.
 */
R8bResamplerHandle r8b_create_resampler(double input_rate, double output_rate, int a_max_in_len, double req_trans_band);
int r8b_process_resampler(R8bResamplerHandle handle, const float* input, int input_frames, float* output, int output_capacity);
void r8b_destroy_resampler(R8bResamplerHandle handle);
void r8b_resampler_clear(R8bResamplerHandle handle);
int r8b_resampler_get_latency(R8bResamplerHandle handle);
double r8b_resampler_get_latency_frac(R8bResamplerHandle handle);
int r8b_resampler_get_max_out_len(R8bResamplerHandle handle, int max_in_len);
int r8b_resampler_get_in_len_before_out_pos(R8bResamplerHandle handle, int req_out_pos);

// Stage reflection
int r8b_resampler_get_stage_count(R8bResamplerHandle handle);
const char* r8b_resampler_get_stage_name(R8bResamplerHandle handle, int stage_index);
double r8b_resampler_get_stage_latency_frac(R8bResamplerHandle handle, int stage_index);
int r8b_resampler_get_stage_in_len_before_out_pos(R8bResamplerHandle handle, int stage_index, int req_out_pos);
int r8b_resampler_get_stage_kernel_len(R8bResamplerHandle handle, int stage_index);
int r8b_resampler_get_stage_input_len(R8bResamplerHandle handle, int stage_index);

// --- Component level verification API ---

R8bProcessorHandle r8b_create_hb_upsampler(double req_atten, int steep_index, int is_third, double prev_latency, int do_consume_latency);
R8bProcessorHandle r8b_create_hb_downsampler(double req_atten, int steep_index, int is_third, double prev_latency);
R8bProcessorHandle r8b_create_frac_interpolator(double src_rate, double dst_rate, double req_atten, int is_third, double prev_latency);

// BlockConvolver needs a filter
R8bFilterHandle r8b_create_lp_filter(double norm_freq, double trans_band, double req_atten, int phase_type, double up_factor);
void r8b_delete_filter(R8bFilterHandle filter);
R8bProcessorHandle r8b_create_block_convolver(R8bFilterHandle filter, int up_factor, int down_factor, double prev_latency, int do_consume_latency);

// Generic processor methods
int r8b_process_processor(R8bProcessorHandle handle, const double* input, int input_frames, double* output, int output_capacity);
void r8b_destroy_processor(R8bProcessorHandle handle);
void r8b_processor_clear(R8bProcessorHandle handle);
int r8b_processor_get_latency(R8bProcessorHandle handle);
double r8b_processor_get_latency_frac(R8bProcessorHandle handle);
int r8b_processor_get_max_out_len(R8bProcessorHandle handle, int max_in_len);

#ifdef __cplusplus
}
#endif

#endif // R8BRAIN_WRAPPER_H