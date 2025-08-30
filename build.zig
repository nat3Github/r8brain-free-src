const std = @import("std");
const update = @import("update_tool");
const deps: []const update.GitDependency = &.{
    .{
        // update self
        .url = "https://github.com/nat3Github/zig-lib-update",
        .branch = "main",
    },
    // .{
    //     // update pffft
    //     .url = "https://github.com/nat3Github/zig-lib-pffft",
    //     .branch = "zig",
    // },
};

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    if (update.updateDependencies(b, deps, .{
        .name = "update",
        .optimize = optimize,
        .target = target,
    })) return;

    const pffft_dep = b.dependency("pffft", .{ .optimize = optimize, .target = target });
    const pffft_mod = pffft_dep.module("pffft");

    const translate_c = b.addTranslateC(.{ .optimize = optimize, .target = target, .root_source_file = b.path("zig-src/r8brain_wrapper.h") });
    const c_mod = translate_c.createModule();
    c_mod.addCSourceFiles(
        .{
            // NOTE: r8brain is compiled with pffft but not with the version supplied in this repository
            // instead it uses a more recent version through a zig dependency
            // the files are therefore commented out (to avoid linking issues)
            // if you depend on pffft in other parts of your programm you have to decide on a common pffft implemenatation to avoid linking issues
            .files = &.{
                "r8bbase.cpp",
                // "pffft.cpp",
                // "pffft_double/pffft_double.c",
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
    c_mod.addIncludePath(b.path(""));
    c_mod.addIncludePath(b.path("zig-src"));
    c_mod.link_libc = true;
    c_mod.link_libcpp = true;
    if (target.result.os.tag == .windows) {
        c_mod.linkSystemLibrary("kernel32", .{});
    }
    if (target.result.os.tag == .macos or target.result.os.tag == .linux) {
        c_mod.linkSystemLibrary("pthread", .{});
    }

    const module = b.addModule("r8brain", .{
        .root_source_file = b.path("zig-src/root.zig"),
        .target = target,
        .optimize = optimize,
    });
    module.addImport("c", c_mod);
    // we include pffft through our dependency
    module.addImport("pffft", pffft_mod);

    try update.addTestFolder(b, "zig-src/tests", optimize, target, &.{.{ .mod = module, .name = "r8brain" }}, "test");
}
