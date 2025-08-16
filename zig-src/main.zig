const std = @import("std");
const print = std.debug.print;
const ArrayList = std.ArrayList;
const R8bResampler = @import("r8brain");
pub fn main() !void {
    print("Zig r8brain-free-src Resampler Example (Simplified Main.zig)\n", .{});
    const input_rate: f64 = 44100.0;
    const output_rate: f64 = 48000.0;
    const freq: f64 = 440.0; // A4 note (for sine wave generation)

    const max_input_frames_chunk = 44100; // Process in chunks of this size
    const req_trans_band: f64 = 2.0;

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    // Create a resampler instance
    var resampler = try R8bResampler.init(input_rate, output_rate, max_input_frames_chunk, req_trans_band);
    defer resampler.deinit();

    print("\n--- Resampler Info ---\n", .{});
    print("  Latency (Integer): {d} samples\n", .{resampler.getLatency()});
    print("  Fractional Latency: {d:.4} samples\n", .{resampler.getLatencyFrac()});
    print("  Max Output Length for {d} input frames chunk: {d}\n", .{ max_input_frames_chunk, resampler.getMaxOutLen(@intCast(max_input_frames_chunk)) });
    var output_frames = ArrayList(f32).init(allocator);
    defer output_frames.deinit();

    var input = ArrayList(f32).init(allocator);
    defer input.deinit();
    for (0..44100) |i| {
        const t = @as(f64, @floatFromInt(i)) / input_rate;
        const sinef = @as(f32, @floatCast(std.math.sin(2 * std.math.pi * freq * t)));
        try input.append(sinef);
    }

    const wbuff = try allocator.alloc(f32, 44100 * 2);
    defer allocator.free(wbuff);

    const produced = try resampler.process(input.items, wbuff);
    try output_frames.appendSlice(produced);

    print("Draining resampler for remaining output...\n", .{});
    while (true) {
        const p = try resampler.process(&[_]f32{}, wbuff); // Pass empty input slice
        if (p.len == 0) {
            break; // No more output
        }
        try output_frames.appendSlice(p);
    }

    print("\nResampling complete. Total output frames collected: {d}\n", .{output_frames.items.len});

    // Display a few samples for verification
    print("\nFirst 10 output samples:\n", .{});
    for (0..@min(10, output_frames.items.len)) |i| {
        print("  [{d}]: {d:.4}\n", .{ i, output_frames.items[i] });
    }

    print("\nLast 10 output samples (if available):\n", .{});
    const start_idx = if (output_frames.items.len > 10) output_frames.items.len - 10 else 0;
    for (start_idx..output_frames.items.len) |i| {
        print("  [{d}]: {d:.4}\n", .{ i, output_frames.items[i] });
    }
}
