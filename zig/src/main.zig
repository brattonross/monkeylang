const std = @import("std");
const Lexer = @import("./Lexer.zig");
const Parser = @import("./Parser.zig");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer std.debug.assert(gpa.deinit() == .ok);

    const allocator = gpa.allocator();

    var root_arena = std.heap.ArenaAllocator.init(allocator);
    defer root_arena.deinit();

    const root_alloc = root_arena.allocator();

    var args = try std.process.argsWithAllocator(root_alloc);
    const exe = args.next() orelse unreachable; // ignore exe path

    const filename = args.next() orelse {
        std.log.err("usage: {s} <filename>", .{exe});
        std.process.exit(1);
    };

    const cwd = std.fs.cwd();
    var file = try cwd.openFile(filename, .{});
    defer file.close();

    const input = try file.readToEndAlloc(root_alloc, 4096);
    var lexer = Lexer.init(input);
    var parser = try Parser.init(root_alloc, &lexer);
    const program = try parser.parseProgram();

    for (parser.errors.items) |err| {
        std.log.err("{s}", .{err});
    }

    std.debug.print("{}\n", .{program});
}
