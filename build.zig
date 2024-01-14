const std = @import("std");
const builtin = @import("builtin");
const app_name = "crosswire_editor";

const release_flags = &[_][]const u8{
    "-DNDEBUG",
    "-std=c++20",
    "-DZIGBUILD",
};
const debug_flags = &[_][]const u8{
    "-g",
    "-std=c++20",
    "-DZIGBUILD",
};

const zcc = @import("compile_commands");

const cpp_sources = &[_][]const u8{
    "src/main.cpp",
    "src/Polygon.cpp",
    "src/util.cpp",
    "src/ImageSelector.cpp",
    "src/Room.cpp",
};

const include_dirs = &[_][]const u8{
    "src/",
};

pub fn build(b: *std.Build) !void {
    // options
    const target = b.standardTargetOptions(.{});
    const mode = b.standardOptimizeOption(.{});

    var flags = std.ArrayList([]const u8).init(b.allocator);
    defer flags.deinit();
    try flags.appendSlice(if (mode == .Debug) debug_flags else release_flags);

    var targets = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
    defer targets.deinit();

    var sources = std.ArrayList([]const u8).init(b.allocator);
    defer sources.deinit();

    try sources.appendSlice(cpp_sources);

    // create executable
    var exe: *std.Build.CompileStep =
        b.addExecutable(.{
        .name = app_name,
        .optimize = mode,
        .target = target,
    });
    try targets.append(exe);
    b.installArtifact(exe);

    exe.linkLibCpp();

    exe.linkSystemLibrary("SDL2");
    exe.linkSystemLibrary("SDL2_image");

    const imgui = b.dependency("imgui", .{
        .target = target,
        .optimize = mode,
        .backend = .SDL2,
    });

    exe.step.dependOn(imgui.builder.getInstallStep());

    exe.addIncludePath(.{
        .path = b.pathJoin(&.{ imgui.builder.install_prefix, "include" }),
    });

    const getSources = @import("imgui").getSources;
    for (getSources(b, .SDL2) catch @panic("OOM")) |src| {
        try sources.append(b.pathJoin(&.{ imgui.builder.install_prefix, "src", std.fs.path.basename(src) }));
    }

    for (include_dirs) |include_dir| {
        try flags.append(b.fmt("-I{s}", .{include_dir}));
    }

    {
        const flags_owned = flags.toOwnedSlice() catch @panic("OOM");
        exe.addCSourceFiles(try sources.toOwnedSlice(), flags_owned);
    }

    b.getInstallStep().dependOn(&b.addInstallHeaderFile("src/serialize.h", "crosswire_editor/serialize.h").step);
    b.getInstallStep().dependOn(&b.addInstallHeaderFile("src/Vec2.h", "crosswire_editor/Vec2.h").step);
    b.getInstallStep().dependOn(&b.addInstallHeaderFile("src/terrain.h", "crosswire_editor/terrain.h").step);

    // add "zig build run"
    {
        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }
        const run_step = b.step("run", "Run the app");
        run_step.dependOn(&run_cmd.step);
    }

    // windows requires that no targets use pkg-config. of course.
    // because its a unix thing.
    switch (target.getOsTag()) {
        .windows => for (targets.items) |t| {
            unsetPkgConfig(t);
        },
        else => {},
    }

    zcc.createStep(b, "cdb", try targets.toOwnedSlice());
}

// Recursively unset all link objects' use_pkg_config setting
// fix for https://github.com/ziglang/zig/issues/14341
fn unsetPkgConfig(compile: *std.Build.Step.Compile) void {
    for (compile.link_objects.items) |*lo| {
        switch (lo.*) {
            .system_lib => |*system_lib| {
                system_lib.use_pkg_config = .no;
            },
            .other_step => |child_compile| unsetPkgConfig(child_compile),
            else => {},
        }
    }
}
