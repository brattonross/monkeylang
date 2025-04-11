allocator: Allocator,
store: std.StringHashMap(Object),
outer: ?*Environment = null,

pub fn init(allocator: Allocator) Environment {
    return .{ .allocator = allocator, .store = std.StringHashMap(Object).init(allocator) };
}

pub fn deinit(self: *Environment) void {
    var keys = self.store.keyIterator();
    while (keys.next()) |key_ptr| {
        self.allocator.free(key_ptr.*);
    }
    self.store.deinit();
}

pub fn get(self: Environment, key: []const u8) ?Object {
    return self.store.get(key) orelse if (self.outer) |outer| outer.get(key) else null;
}

pub fn set(self: *Environment, key: []const u8, value: Object) !void {
    const k = try self.allocator.dupe(u8, key);
    try self.store.put(k, value);
}

const Environment = @This();

const std = @import("std");
const Allocator = std.mem.Allocator;
const Object = @import("./object.zig").Object;
