const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const module = b.addModule("r8brain", .{
        .root_source_file = b.path("zig-src/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    module.addCSourceFiles(
        .{
            .files = &.{
                "r8bbase.cpp",
                "pffft.cpp",
                // Also add your C++ wrapper source file.
                "zig-src/r8brain_wrapper.cpp",
            },
            .language = .cpp,
            .flags = &.{
                "-std=c++11",
                "-fno-exceptions",
                "-fno-rtti",
            },
        },
    );
    module.addIncludePath(b.path(""));
    module.addIncludePath(b.path("zig-src"));

    module.link_libc = true;
    module.link_libcpp = true;

    if (target.result.os.tag == .windows) {
        module.linkSystemLibrary("kernel32", .{});
    }
    if (target.result.os.tag == .macos or target.result.os.tag == .linux) {
        module.linkSystemLibrary("pthread", .{});
    }

    const exe = b.addExecutable(.{
        .name = "r8brain-zig-example",
        .root_source_file = b.path("zig-src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    exe.root_module.addImport("r8brain", module);
    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the example");
    run_step.dependOn(&run_cmd.step);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe.step);
}
