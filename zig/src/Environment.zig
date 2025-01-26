const std = @import("std");
const Allocator = std.mem.Allocator;
const Object = @import("./Evaluator.zig").Object;

const Environment = @This();

allocator: Allocator,
store: std.StringHashMap(Object),

pub fn init(allocator: Allocator) Environment {
    return .{ .allocator = allocator, .store = std.StringHashMap(Object).init(allocator) };
}

pub fn get(self: Environment, key: []const u8) ?Object {
    return self.store.get(key);
}

pub fn set(self: *Environment, key: []const u8, value: Object) !void {
    const k = try self.allocator.dupe(u8, key);
    try self.store.put(k, value);
}
