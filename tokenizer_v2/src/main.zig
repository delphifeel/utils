const std = @import("std");
const debug = std.debug;
const fmt = std.fmt;
const mem = std.mem;
const Allocator = std.mem.Allocator;
const string_view = []const u8;
const string = []u8;

fn oom() noreturn {
    debug.panic("Out Of Memory\n", .{});
}

fn readFile(allocator: Allocator, file_name: string_view) ![]string_view {
    var file = try std.fs.cwd().openFile(file_name, .{});
    defer file.close();

    var buffered = std.io.bufferedReader(file.reader());
    var reader = buffered.reader();
    var buf: [2048]u8 = undefined;
    var list = std.ArrayList([]u8).init(allocator);
    errdefer list.deinit();
    while (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line_raw| {
        var line_copy = try allocator.dupe(u8, line_raw);
        try list.append(line_copy);
    }

    return try list.toOwnedSlice();
}

fn printWrongUsage() void {
    debug.print("Wrong usage\n", .{});
}

const ListItem = union(enum) {
    s: string_view,
    list: []ListItem,
};

const State = struct {
    // next function call arguments
    args: std.ArrayList(string_view),
    data_list: ListItem,
    data_list_orig: string_view,
};

fn printDataList(data_list: *const ListItem) void {
    switch (data_list.*) {
        ListItem.s => |s| {
            debug.print("```\n{s}\n```\n", .{s});
        },
        ListItem.list => |ls| {
            debug.print("[ ", .{});
            for (ls) |*l| {
                printDataList(l);
            }
            debug.print("]", .{});
        },
    }
}

// @0 split_by sequence_of ({
// @1 split_by any_of :,
fn splitBy(allocator: Allocator, state: *State) void {
    var in_arg = state.args.items[0];
    if (in_arg[0] != '@') {
        debug.print("Unknown arg #1\n", .{});
        return;
    }
    if (in_arg.len < 2) {
        std.debug.print("arg #1 is too small\n", .{});
        return;
    }

    in_arg = in_arg[1..];

    var first_time = false;
    var str_to_process: string_view = undefined;
    var i: u32 = undefined;
    defer allocator.free(str_to_process);
    if (in_arg[0] == '0') {
        first_time = true;
        var orig_copy = allocator.dupe(u8, state.data_list_orig) catch oom();
        str_to_process = orig_copy;
    } else {
        i = fmt.parseUnsigned(u32, in_arg, 10) catch unreachable;
        switch (state.data_list) {
            ListItem.s => unreachable,
            ListItem.list => |list| {
                switch (list[i]) {
                    ListItem.s => |s| str_to_process = s,
                    else => unreachable,
                }
            },
        }
    }

    var split_type = state.args.items[1];
    var tokens = state.args.items[2];
    var new_list_arr = std.ArrayList(string_view).init(allocator);
    defer new_list_arr.deinit();
    if (mem.eql(u8, split_type, "seq")) {
        var iter = mem.tokenizeSequence(u8, str_to_process, tokens);
        while (iter.next()) |part_of_s| {
            new_list_arr.append(allocator.dupe(u8, part_of_s) catch oom()) catch oom();
        }
    } else if (mem.eql(u8, split_type, "any")) {
        var iter = mem.tokenizeAny(u8, str_to_process, tokens);
        while (iter.next()) |part_of_s| {
            new_list_arr.append(allocator.dupe(u8, part_of_s) catch oom()) catch oom();
        }
    }

    if (first_time) {
        var v = allocator.alloc(ListItem, new_list_arr.items.len) catch oom();
        state.data_list = .{ .list = v };
        for (new_list_arr.items, 0..) |str, s_i| {
            state.data_list.list[s_i] = .{ .s = str };
        }
    } else {
        var v = allocator.alloc(ListItem, new_list_arr.items.len) catch oom();
        state.data_list.list[i] = .{ .list = v };
        for (new_list_arr.items, 0..) |str, s_i| {
            state.data_list.list[i].list[s_i] = .{ .s = str };
        }
    }
}

fn processInput(allocator: Allocator, command: string_view, state: *State) void {
    defer state.args.clearRetainingCapacity();

    var tokens_iter = mem.tokenizeScalar(u8, command, ' ');
    // arg1 func_name arg2 argN
    state.args.append(tokens_iter.next().?) catch oom();
    var function_name = tokens_iter.next() orelse {
        printWrongUsage();
        return;
    };

    while (tokens_iter.next()) |arg| {
        state.args.append(arg) catch oom();
    }

    // check functions
    if (mem.eql(u8, function_name, "read_file")) {
        var lines = readFile(allocator, state.args.items[0]) catch {
            debug.print("read_file error\n", .{});
            return;
        };
        var str = std.ArrayList(u8).init(allocator);
        for (lines) |line| {
            str.appendSlice(line) catch oom();
            str.append('\n') catch oom();
        }
        var s = str.toOwnedSlice() catch oom();
        state.data_list_orig = allocator.dupe(u8, s) catch oom();
        state.data_list.s = s;
        return;
    }
    if (mem.eql(u8, function_name, "print")) {
        printDataList(&state.data_list);
        debug.print("\n", .{});
        return;
    }
    if (mem.eql(u8, function_name, "spl")) {
        splitBy(allocator, state);
        return;
    }
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    debug.assert(gpa.detectLeaks() == false);
    var allocator = gpa.allocator();

    const stdin = std.io.getStdIn().reader();
    var buf: [1024]u8 = undefined;

    var state = State{
        .args = std.ArrayList(string_view).init(allocator),
        .data_list = .{ .s = "" },
        .data_list_orig = "",
    };
    defer state.args.deinit();

    while (true) {
        debug.print("> ", .{});
        var command = try stdin.readUntilDelimiterOrEof(&buf, '\n') orelse continue;
        processInput(allocator, command, &state);
    }
}

test "simple test" {}
