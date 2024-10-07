const std = @import("std");
const Lexer = @import("Lexer.zig");
const Token = @import("Token.zig");

const prompt = ">> ";

pub fn main() !void {
    const stdin = std.io.getStdIn().reader();
    const stdout = std.io.getStdOut().writer();

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer std.debug.assert(gpa.deinit() == .ok);

    const alloc = gpa.allocator();
    const username = try std.process.getEnvVarOwned(alloc, "USER");
    defer alloc.free(username);

    try stdout.print("Hello {s}! This is the Monkey programming language!\n", .{username});
    try stdout.print("Feel free to type in commands\n", .{});

    while (true) {
        try stdout.print("{s}", .{prompt});
        var buf: [1024]u8 = undefined;
        if (try stdin.readUntilDelimiterOrEof(&buf, '\n')) |input| {
            var lexer = Lexer.init(input);
            var token = lexer.nextToken();
            while (token.type != .eof) : (token = lexer.nextToken()) {
                try stdout.print("{}\n", .{token});
            }
        }
    }
}
