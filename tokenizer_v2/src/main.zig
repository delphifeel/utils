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
    allocator: Allocator,
    // next function call arguments
    args: std.ArrayList(string_view),
    data_list: ListItem,

    pub fn init(allocator: Allocator) State {
        return .{
            .allocator = allocator,
            .args = std.ArrayList(string_view).init(allocator),
            .data_list = .{ .s = allocator.alloc(u8, 1) catch oom() },
        };
    }
};

fn printDataList(data_list: *const ListItem) void {
    switch (data_list.*) {
        ListItem.s => |s| {
            if (s.len <= 30) {
                debug.print("\"{s}\", ", .{s});
            } else {
                debug.print("\"{s}...\", ", .{s[0..30]});
            }
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

fn isOrigArg(in_arg: string_view) bool {
    if (in_arg.len != 2) {
        return false;
    }
    return in_arg[0] == '@' and in_arg[1] == '0';
}

fn getArgListItem(allocator: Allocator, arg: string_view, state: *State) ?*ListItem {
    _ = allocator;
    if (arg[0] != '@') {
        debug.print("Unknown arg #1\n", .{});
        return null;
    }
    if (arg.len < 2) {
        std.debug.print("arg #1 is too small\n", .{});
        return null;
    }

    var in_arg = arg[1..];

    var str_to_process: string_view = undefined;
    _ = str_to_process;
    var i: u32 = undefined;

    i = fmt.parseUnsigned(u32, in_arg, 10) catch {
        debug.print("Error parsing arg: {s}\n", .{in_arg});
        return null;
    };
    if (i == 0) {
        debug.print("Wrong arg: got @0 expected @n(n > 0)\n", .{});
        return null;
    }
    switch (state.data_list) {
        ListItem.s => {
            debug.print("Wrong arg type: got string, expected list\n", .{});
            return null;
        },
        ListItem.list => |list| {
            if (list.len < i) {
                debug.print("Wrong arg: list len < {}\n", .{i});
                return null;
            }
            return &list[i - 1];
            // switch (list[i]) {
            //     ListItem.s => |s| str_to_process = s,
            //     else => {
            //         debug.print("Wrong arg type: got list, expected string\n", .{});
            //         return null;
            //     },
            // }
        },
    }
}

// Examples:
// - @0 split_by sequence_of ({
// - @1 split_by any_of :,
fn splitBy(allocator: Allocator, state: *State) void {
    if (state.args.items.len < 3) {
        debug.print("Not enough args. Expected 3\n", .{});
        return;
    }

    var in_arg = state.args.items[0];
    var is_orig_arg = isOrigArg(in_arg);
    var str_to_process: string_view = undefined;
    var list_to_update: *ListItem = undefined;
    if (is_orig_arg) {
        list_to_update = &state.data_list;
    } else {
        list_to_update = getArgListItem(allocator, in_arg, state) orelse return;
    }

    switch (list_to_update.*) {
        ListItem.s => |s| {
            str_to_process = s;
        },
        else => {
            debug.print("Wrong arg type: got list, expected string\n", .{});
            return;
        },
    }

    defer allocator.free(str_to_process);

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

    var v = allocator.alloc(ListItem, new_list_arr.items.len) catch oom();
    list_to_update.* = .{ .list = v };

    for (new_list_arr.items, 0..) |str, s_i| {
        list_to_update.list[s_i] = .{ .s = str };
    }
}

// Leave only 1 list item
// Example:
// - @1 take
fn take(allocator: Allocator, state: *State) void {
    var list_item = getArgListItem(allocator, state.args.items[0], state) orelse return;
    // we need free all items except we take
    switch (state.data_list) {
        ListItem.list => |ls| {
            for (ls) |*l| {
                if (l != list_item) {
                    clearListItem(allocator, l);
                }
            }
            //allocator.free(ls);
        },
        ListItem.s => unreachable,
    }

    state.data_list = list_item.*;
}

fn clearListItem(allocator: Allocator, list_item: *ListItem) void {
    switch (list_item.*) {
        ListItem.list => |ls| {
            for (ls) |*l| {
                clearListItem(allocator, l);
            }
            allocator.free(ls);
        },
        ListItem.s => |s| {
            _ = s;
            //allocator.free(s);
        },
    }
}

fn processInput(allocator: Allocator, command: string_view, state: *State) void {
    defer state.args.clearRetainingCapacity();

    var tokens_iter = mem.tokenizeScalar(u8, command, ' ');
    // parse args
    // command have format:
    // - arg1 func_name arg2 argN
    state.args.append(tokens_iter.next().?) catch oom();
    var function_name = tokens_iter.next() orelse {
        printWrongUsage();
        return;
    };

    while (tokens_iter.next()) |arg| {
        state.args.append(arg) catch oom();
    }

    // call specific function
    if (mem.eql(u8, function_name, "fread")) {
        var lines = readFile(allocator, state.args.items[0]) catch {
            debug.print("fread error\n", .{});
            return;
        };
        defer {
            for (lines) |line| {
                allocator.free(line);
            }
            allocator.free(lines);
        }
        var str = std.ArrayList(u8).init(allocator);
        for (lines) |line| {
            str.appendSlice(line) catch oom();
            str.append('\n') catch oom();
        }
        var s = str.toOwnedSlice() catch oom();
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
    if (mem.eql(u8, function_name, "take")) {
        take(allocator, state);
        return;
    }
}

fn init(allocator: Allocator) !void {
    const stdin = std.io.getStdIn().reader();
    var buf: [1024]u8 = undefined;

    var state = State.init(allocator);
    defer state.args.deinit();

    while (true) {
        debug.print("> ", .{});
        var command = try stdin.readUntilDelimiterOrEof(&buf, '\n') orelse continue;

        if (command[0] == 'q') {
            break;
        }

        processInput(allocator, command, &state);
    }
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    debug.assert(gpa.detectLeaks() == false);
    var allocator = gpa.allocator();
    try init(allocator);
}

test "simple test" {
    try init(std.testing.allocator);
}
