pub fn len(ctx: *anyopaque, args: []Object) BuiltinError!Object {
    const self: *Builtin = @ptrCast(@alignCast(ctx));

    if (args.len != 1) {
        const message = try std.fmt.allocPrint(self.allocator, "wrong number of arguments. got={d}, want=1", .{args.len});
        return .{ .@"error" = .{ .message = message } };
    }

    const arg = args[0];
    return switch (arg) {
        .string => |s| .{ .integer = .{ .value = @intCast(s.value.len) } },
        .array => |arr| .{ .integer = .{ .value = @intCast(arr.elements.items.len) } },
        else => blk: {
            const message = try std.fmt.allocPrint(self.allocator, "argument to `len` not supported, got {s}", .{@tagName(args[0])});
            break :blk .{ .@"error" = .{ .message = message } };
        },
    };
}

pub fn first(ctx: *anyopaque, args: []Object) BuiltinError!Object {
    const self: *Builtin = @ptrCast(@alignCast(ctx));

    if (args.len != 1) {
        const message = try std.fmt.allocPrint(self.allocator, "wrong number of arguments. got={d}, want=1", .{args.len});
        return .{ .@"error" = .{ .message = message } };
    }

    const arg = args[0];
    return switch (arg) {
        .array => |arr| if (arr.elements.items.len > 0) arr.elements.items[0] else null_object,
        else => blk: {
            const message = try std.fmt.allocPrint(self.allocator, "argument to `first` not supported, got {s}", .{@tagName(args[0])});
            break :blk .{ .@"error" = .{ .message = message } };
        },
    };
}

pub fn last(ctx: *anyopaque, args: []Object) BuiltinError!Object {
    const self: *Builtin = @ptrCast(@alignCast(ctx));

    if (args.len != 1) {
        const message = try std.fmt.allocPrint(self.allocator, "wrong number of arguments. got={d}, want=1", .{args.len});
        return .{ .@"error" = .{ .message = message } };
    }

    const arg = args[0];
    return switch (arg) {
        .array => |arr| if (arr.elements.items.len > 0) arr.elements.items[arr.elements.items.len - 1] else null_object,
        else => blk: {
            const message = try std.fmt.allocPrint(self.allocator, "argument to `last` not supported, got {s}", .{@tagName(args[0])});
            break :blk .{ .@"error" = .{ .message = message } };
        },
    };
}

pub fn tail(ctx: *anyopaque, args: []Object) BuiltinError!Object {
    const self: *Builtin = @ptrCast(@alignCast(ctx));

    if (args.len != 1) {
        const message = try std.fmt.allocPrint(self.allocator, "wrong number of arguments. got={d}, want=1", .{args.len});
        return .{ .@"error" = .{ .message = message } };
    }

    const arg = args[0];
    return switch (arg) {
        .array => |arr| blk: {
            if (arr.elements.items.len > 0) {
                var new_arr = try std.ArrayList(Object).initCapacity(self.allocator, arr.elements.items.len - 1);
                try new_arr.appendSlice(arr.elements.items[1..]);
                break :blk .{ .array = .{ .elements = new_arr } };
            }
            return null_object;
        },
        else => blk: {
            const message = try std.fmt.allocPrint(self.allocator, "argument to `tail` not supported, got {s}", .{@tagName(args[0])});
            break :blk .{ .@"error" = .{ .message = message } };
        },
    };
}

pub fn push(ctx: *anyopaque, args: []Object) BuiltinError!Object {
    const self: *Builtin = @ptrCast(@alignCast(ctx));

    if (args.len != 2) {
        const message = try std.fmt.allocPrint(self.allocator, "wrong number of arguments. got={d}, want=2", .{args.len});
        return .{ .@"error" = .{ .message = message } };
    }

    const arg = args[0];
    return switch (arg) {
        .array => |arr| blk: {
            var new_arr = try std.ArrayList(Object).initCapacity(self.allocator, arr.elements.items.len + 1);
            try new_arr.appendSlice(arr.elements.items);
            try new_arr.append(args[1]);
            break :blk .{ .array = .{ .elements = new_arr } };
        },
        else => blk: {
            const message = try std.fmt.allocPrint(self.allocator, "argument to `push` not supported, got {s}", .{@tagName(args[0])});
            break :blk .{ .@"error" = .{ .message = message } };
        },
    };
}

pub fn puts(ctx: *anyopaque, args: []Object) BuiltinError!Object {
    const self: *Builtin = @ptrCast(@alignCast(ctx));

    for (args) |arg| {
        try self.stdout.print("{}", .{arg});
    }
    try self.stdout.writeAll("\n");
    return null_object;
}

const std = @import("std");
const object = @import("./object.zig");
const Builtin = object.Builtin;
const BuiltinError = object.BuiltinError;
const Object = object.Object;
const null_object = object.null_object;
