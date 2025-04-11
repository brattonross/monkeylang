pub fn len(ctx: *anyopaque, args: []Object) !Object {
    const self: *Builtin = @ptrCast(@alignCast(ctx));

    if (args.len != 1) {
        const message = try std.fmt.allocPrint(self.allocator, "wrong number of arguments. got={d}, want=1", .{args.len});
        return .{ .@"error" = .{ .message = message } };
    }

    return switch (args[0]) {
        .string => .{ .integer = .{ .value = @intCast(args[0].string.value.len) } },
        else => blk: {
            const message = try std.fmt.allocPrint(self.allocator, "argument to `len` not supported, got {s}", .{@tagName(args[0])});
            break :blk .{ .@"error" = .{ .message = message } };
        },
    };
}

const std = @import("std");
const object = @import("./object.zig");
const Builtin = object.Builtin;
const Object = object.Object;
