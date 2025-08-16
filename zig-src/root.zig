const std = @import("std");
pub const c = @import("c");
pub const pffft = @import("pffft");

pub const R8bResampler = @This();

handle: c.R8bResamplerHandle,
pub fn init(input_rate: f64, output_rate: f64, max_input_frames_chunk: u32, req_trans_band: f64) !R8bResampler {
    _ = pffft;
    const handle = c.r8b_create_resampler(input_rate, output_rate, @intCast(max_input_frames_chunk), req_trans_band);
    if (handle == null) {
        return error.ResamplerCreationFailed;
    }
    return .{
        .handle = handle,
    };
}

pub fn process(self: *R8bResampler, input: []const f32, output: []f32) ![]const f32 {
    const produced_frames = c.r8b_process_resampler(
        self.handle,
        input.ptr, // Pointer to input data
        @intCast(input.len), // Number of input frames
        output.ptr,
        @intCast(output.len),
    );
    if (produced_frames < 0) {
        return error.ResamplingFailed;
    }
    return output[0..@intCast(produced_frames)];
}

/// Deinitializes the R8bResampler instance, freeing resources.
pub fn deinit(self: *R8bResampler) void {
    c.r8b_destroy_resampler(self.handle);
    self.handle = null; // Clear the handle to prevent double-free
}

/// Clears the internal buffers and resets the state of the resampler.
pub fn clear(self: *const R8bResampler) void {
    c.r8b_resampler_clear(self.handle);
}

/// Returns the number of input frames before a specified output position.
pub fn getInLenBeforeOutPos(self: *const R8bResampler, req_out_pos: i32) i32 {
    return c.r8b_resampler_get_in_len_before_out_pos(self.handle, req_out_pos);
}

/// Gets the total latency of the resampler in samples (integer part).
pub fn getLatency(self: *const R8bResampler) i32 {
    return c.r8b_resampler_get_latency(self.handle);
}

/// Gets the fractional part of the resampler's latency.
pub fn getLatencyFrac(self: *const R8bResampler) f64 {
    return c.r8b_resampler_get_latency_frac(self.handle);
}

/// Returns the maximum possible number of output frames that can be generated
/// for a given maximum number of input frames.
pub fn getMaxOutLen(self: *const R8bResampler, max_in_len: i32) i32 {
    return c.r8b_resampler_get_max_out_len(self.handle, max_in_len);
}
