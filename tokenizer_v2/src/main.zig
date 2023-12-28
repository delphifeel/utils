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
    var list = std.ArrayList(string).init(allocator);
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

    pub fn deinit(self: *State) void {
        self.args.deinit();
        clearListItem(self.allocator, &self.data_list);
    }
};

const DEFAULT_MAX_STR_SIZE = 30;

fn printListItem(data_list: *const ListItem, max_str_size: ?u32) void {
    switch (data_list.*) {
        ListItem.s => |s| {
            var max_str_size_v = max_str_size orelse {
                debug.print("\"{s}\", ", .{s});
                return;
            };
            if (s.len <= max_str_size_v) {
                debug.print("\"{s}\", ", .{s});
            } else {
                debug.print("\"{s}...\", ", .{s[0..max_str_size_v]});
            }
        },
        ListItem.list => |ls| {
            debug.print("[ ", .{});
            for (ls) |*l| {
                printListItem(l, max_str_size);
            }
            debug.print("]", .{});
        },
    }
}

fn printState(state: *const State) void {
    var in_arg = state.args.items[0];
    var max_str_size: ?u32 = if (state.args.items.len > 1 and mem.eql(u8, state.args.items[1], "full")) null else DEFAULT_MAX_STR_SIZE;
    if (isWholeDataListArg(in_arg)) {
        printListItem(&state.data_list, max_str_size);
        debug.print("\n", .{});
        return;
    }

    var list_item = getArgListItem(state.allocator, in_arg, state) orelse return;
    printListItem(list_item, max_str_size);
    debug.print("\n", .{});

    return;
}

fn isWholeDataListArg(in_arg: string_view) bool {
    if (in_arg.len != 2) {
        return false;
    }
    return in_arg[0] == '@' and in_arg[1] == '0';
}

fn getArgListItem(allocator: Allocator, arg: string_view, state: *const State) ?*ListItem {
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
    var is_whole_data_list = isWholeDataListArg(in_arg);
    var str_to_process: string_view = undefined;
    var list_to_update: *ListItem = undefined;
    if (is_whole_data_list) {
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
            var s_trimmed = mem.trim(u8, part_of_s, "\n\r\t ");
            new_list_arr.append(s_trimmed) catch oom();
        }
    } else if (mem.eql(u8, split_type, "any")) {
        var iter = mem.tokenizeAny(u8, str_to_process, tokens);
        while (iter.next()) |part_of_s| {
            var s_trimmed = mem.trim(u8, part_of_s, "\n\r\t ");
            new_list_arr.append(s_trimmed) catch oom();
        }
    }

    var v = allocator.alloc(ListItem, new_list_arr.items.len) catch oom();
    list_to_update.* = .{ .list = v };

    for (new_list_arr.items, 0..) |str, s_i| {
        list_to_update.list[s_i] = .{ .s = allocator.dupe(u8, str) catch oom() };
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
            allocator.free(s);
        },
    }
}

fn fileRead(state: *State) void {
    var lines = readFile(state.allocator, state.args.items[0]) catch {
        debug.print("fread error\n", .{});
        return;
    };
    defer {
        for (lines) |line| {
            state.allocator.free(line);
        }
        state.allocator.free(lines);
    }
    var str = std.ArrayList(u8).init(state.allocator);
    for (lines) |line| {
        str.appendSlice(line) catch oom();
        str.append('\n') catch oom();
    }
    var s = str.toOwnedSlice() catch oom();
    state.data_list.s = s;
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
        fileRead(state);
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
    if (mem.eql(u8, function_name, "print")) {
        printState(state);
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
    var allocator = std.testing.allocator;
    const stdin = std.io.getStdIn().reader();
    _ = stdin;
    var buf: [1024]u8 = undefined;
    _ = buf;

    var state = State.init(allocator);
    defer state.deinit();

    var lines = try readFile(allocator, "tests/cmds_1.txt");
    defer {
        for (lines) |line| {
            allocator.free(line);
        }
        allocator.free(lines);
    }
    for (lines) |line| {
        processInput(allocator, line, &state);
    }
}
