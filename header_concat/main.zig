const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    var allocator = gpa.allocator();
    var args = try std.process.argsAlloc(allocator);
    if (args.len < 3) {
        std.debug.print("Wrong usage. Format: ./bin [file_to_read] [file_to_write] v?\n", .{});
        return;
    }

    var file_to_read = args[1];
    // var file_to_write = args[2];

    var f = try std.fs.cwd().openFile(file_to_read, .{});
    defer f.close();

    var buf_r = std.io.bufferedReader(f.reader());
    var in_stream = buf_r.reader();
    var buf: [1024]u8 = undefined;
    while (try in_stream.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        std.debug.print("{s}\n", .{line});
    }
}
