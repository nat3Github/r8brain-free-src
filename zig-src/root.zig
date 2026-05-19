const std = @import("std");

pub const R8bResamplerHandle = ?*anyopaque;
pub const R8bProcessorHandle = ?*anyopaque;
pub const R8bFilterHandle = ?*anyopaque;

pub extern fn r8b_create_resampler(input_rate: f64, output_rate: f64, a_max_in_len: i32, req_trans_band: f64) R8bResamplerHandle;
pub extern fn r8b_process_resampler(handle: R8bResamplerHandle, input: [*]const f32, input_frames: i32, output: [*]f32, output_capacity: i32) i32;
pub extern fn r8b_destroy_resampler(handle: R8bResamplerHandle) void;
pub extern fn r8b_resampler_clear(handle: R8bResamplerHandle) void;
pub extern fn r8b_resampler_get_latency(handle: R8bResamplerHandle) i32;
pub extern fn r8b_resampler_get_latency_frac(handle: R8bResamplerHandle) f64;
pub extern fn r8b_resampler_get_max_out_len(handle: R8bResamplerHandle, max_in_len: i32) i32;
pub extern fn r8b_resampler_get_in_len_before_out_pos(handle: R8bResamplerHandle, req_out_pos: i32) i32;
pub extern fn r8b_resampler_get_stage_count(handle: R8bResamplerHandle) i32;
pub extern fn r8b_resampler_get_stage_name(handle: ?*anyopaque, stage_index: i32) [*:0]const u8;
pub extern fn r8b_resampler_get_stage_latency_frac(handle: ?*anyopaque, stage_index: i32) f64;
pub extern fn r8b_resampler_get_stage_in_len_before_out_pos(handle: ?*anyopaque, stage_index: i32, req_out_pos: i32) i32;
pub extern fn r8b_resampler_get_stage_kernel_len(handle: ?*anyopaque, stage_index: i32) i32;
pub extern fn r8b_resampler_get_stage_input_len(handle: ?*anyopaque, stage_index: i32) i32;

pub const R8bResampler = struct {
    handle: R8bResamplerHandle,

    pub fn init(input_rate: f64, output_rate: f64, a_max_in_len: i32, req_trans_band: f64) !R8bResampler {
        const handle = r8b_create_resampler(input_rate, output_rate, a_max_in_len, req_trans_band);
        if (handle == null) return error.InitializationFailed;
        return .{ .handle = handle };
    }

    pub fn deinit(self: R8bResampler) void {
        r8b_destroy_resampler(self.handle);
    }

    pub fn process(self: R8bResampler, input: []const f32, output: []f32) i32 {
        return r8b_process_resampler(self.handle, input.ptr, @intCast(input.len), output.ptr, @intCast(output.len));
    }

    pub fn getLatency(self: R8bResampler) i32 {
        return r8b_resampler_get_latency(self.handle);
    }
    
    pub fn getLatencyFrac(self: R8bResampler) f64 {
        return r8b_resampler_get_latency_frac(self.handle);
    }

    pub fn getMaxOutLen(self: R8bResampler, max_in_len: i32) i32 {
        return r8b_resampler_get_max_out_len(self.handle, max_in_len);
    }

    pub fn getInLenBeforeOutPos(self: R8bResampler, req_out_pos: i32) i32 {
        return r8b_resampler_get_in_len_before_out_pos(self.handle, req_out_pos);
    }

    pub fn getStageCount(self: R8bResampler) i32 {
        return r8b_resampler_get_stage_count(self.handle);
    }

    pub fn getStageName(self: R8bResampler, stage_index: i32) [:0]const u8 {
        const ptr = r8b_resampler_get_stage_name(self.handle, stage_index);
        return std.mem.span(ptr);
    }

    pub fn getStageLatencyFrac(self: R8bResampler, stage_index: i32) f64 {
        return r8b_resampler_get_stage_latency_frac(self.handle, stage_index);
    }

    pub fn debugPrintPipeline(self: R8bResampler) void {
        const count = self.getStageCount();
        std.debug.print("C++ Reference Resampler Pipeline ({d} stages):\n", .{count});
        var i: i32 = 0;
        while (i < count) : (i += 1) {
            const name = self.getStageName(i);
            const lfrac = self.getStageLatencyFrac(i);
            std.debug.print("  Stage {d}: {s} (LatencyFrac: {d:.6})\n", .{ i, name, lfrac });
        }
    }
};

// --- Component level verification API ---

pub extern fn r8b_create_hb_upsampler(req_atten: f64, steep_index: i32, is_third: i32, prev_latency: f64, do_consume_latency: i32) R8bProcessorHandle;
pub extern fn r8b_create_hb_downsampler(req_atten: f64, steep_index: i32, is_third: i32, prev_latency: f64) R8bProcessorHandle;
pub extern fn r8b_create_frac_interpolator(src_rate: f64, dst_rate: f64, req_atten: f64, is_third: i32, prev_latency: f64) R8bProcessorHandle;

pub extern fn r8b_create_lp_filter(norm_freq: f64, trans_band: f64, req_atten: f64, phase_type: i32, up_factor: f64) R8bFilterHandle;
pub extern fn r8b_delete_filter(filter: R8bFilterHandle) void;
pub extern fn r8b_create_block_convolver(filter: R8bFilterHandle, up_factor: i32, down_factor: i32, prev_latency: f64, do_consume_latency: i32) R8bProcessorHandle;

pub extern fn r8b_process_processor(handle: R8bProcessorHandle, input: [*]const f64, input_frames: i32, output: [*]f64, output_capacity: i32) i32;
pub extern fn r8b_destroy_processor(handle: R8bProcessorHandle) void;
pub extern fn r8b_processor_clear(handle: R8bProcessorHandle) void;
pub extern fn r8b_processor_get_latency(handle: R8bProcessorHandle) i32;
pub extern fn r8b_processor_get_latency_frac(handle: R8bProcessorHandle) f64;
pub extern fn r8b_processor_get_max_out_len(handle: R8bProcessorHandle, max_in_len: i32) i32;
