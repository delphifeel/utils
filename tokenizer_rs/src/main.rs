use std::fs::File;
use std::io::{stdin, stdout, Read, Write};

enum ListItem {
    Str(String),
    List(Vec<ListItem>),
}

struct State {
    args: Vec<String>,
    data_list: ListItem,
}

fn read_file(file_name: &str) -> Option<String> {
    let mut file = match File::open(file_name) {
        Ok(f) => f,
        Err(e) => {
            eprintln!("Error reading file: {}", e);
            return None;
        }
    };
    let mut contents = String::new();
    match file.read_to_string(&mut contents) {
        Err(e) => {
            eprintln!("Error reading file: {}", e);
            return None;
        }
        Ok(_) => {}
    }
    return Some(contents);
}

fn leave_only_arg1(state: &mut State) {
    let Some(arg1) = state.args.get(0) else {
        eprintln!("Arg #1 is missing");
        return;
    };

    let i = match arg1.get(0..1) {
        Some("@") => {
            let Some(s) = arg1.get(1..) else {
                eprintln!("Wrong format for Arg #1: @N expected");
                return;
            };

            let i: usize = s.parse().unwrap();
            match i {
                0 => {
                    eprintln!("@0 only has no effect");
                    return;
                }
                _ => i - 1,
            }
        }
        _ => {
            eprintln!("Wrong format for Arg #1: @N expected");
            return;
        }
    };

    state.data_list = match state.data_list {
        ListItem::Str(_) => {
            eprintln!("Expected list, got string");
            return;
        }
        ListItem::List(ref mut ls) => {
            let a = ls.swap_remove(i);
            a
        }
    }
}

fn get_arg1_list(state: &State) -> Option<&ListItem> {
    let Some(arg1) = state.args.get(0) else {
        eprintln!("Arg #1 is missing");
        return None;
    };

    match arg1.get(0..1) {
        Some("@") => {
            let Some(s) = arg1.get(1..) else {
                eprintln!("Wrong format for Arg #1: @N expected");
                return None;
            };

            let i: usize = s.parse().unwrap();
            match i {
                0 => return Some(&state.data_list),
                _ => {
                    let ListItem::List(ref ls) = state.data_list else {
                        eprintln!("Expected list, got string");
                        return None;
                    };
                    let Some(l) = ls.get(i - 1) else {
                        eprintln!("List {} is missing", i);
                        return None;
                    };
                    return Some(l);
                }
            }
        }
        _ => {
            eprintln!("Wrong format for Arg #1: @N expected");
            return None;
        }
    };
}

fn get_arg1_list_mut(state: &mut State) -> Option<&mut ListItem> {
    let Some(arg1) = state.args.get(0) else {
        eprintln!("Arg #1 is missing");
        return None;
    };

    match arg1.get(0..1) {
        Some("@") => {
            let Some(s) = arg1.get(1..) else {
                eprintln!("Wrong format for Arg #1: @N expected");
                return None;
            };

            let i: usize = s.parse().unwrap();
            match i {
                0 => return Some(&mut state.data_list),
                _ => {
                    let ListItem::List(ref mut ls) = state.data_list else {
                        eprintln!("Expected list, got string");
                        return None;
                    };
                    let Some(l) = ls.get_mut(i - 1) else {
                        eprintln!("List {} is missing", i);
                        return None;
                    };
                    return Some(l);
                }
            }
        }
        _ => {
            eprintln!("Wrong format for Arg #1: @N expected");
            return None;
        }
    };
}

const DEFAULT_MAX_PRINT_STR_SIZE: usize = 30;

fn print_list_item(list_item: &ListItem, max_str_size: Option<usize>) {
    match list_item {
        ListItem::Str(ref s) => match max_str_size {
            None => print!("\"{}\", ", s),
            Some(max_size_v) if max_size_v > s.len() => print!("\"{}\", ", s),
            _ => print!("\"{}...\", ", &s[0..DEFAULT_MAX_PRINT_STR_SIZE]),
        },
        ListItem::List(ref ls) => {
            print!("[ ");
            for l in ls {
                print_list_item(l, max_str_size);
            }
            print!("]");
        }
    }
}

// @1 print full
fn print_data_list(state: &mut State) {
    let Some(list_item) = get_arg1_list(state) else {
        return;
    };

    if state.args.get(1).is_some_and(|s| s == "full") {
        print_list_item(list_item, None);
        println!();
    } else {
        print_list_item(list_item, Some(DEFAULT_MAX_PRINT_STR_SIZE));
        println!();
    }
}

// Example:
// - tests/in_1.txt fread
fn file_read(state: &mut State) {
    let file_content = read_file(match state.args.get(0) {
        Some(s) => s.as_str(),
        None => {
            eprintln!("Arg #1 is missing");
            return;
        }
    });
    match file_content {
        Some(s) => state.data_list = ListItem::Str(s),
        None => return,
    };
}

// Example:
// - @0 spl seq ({
// - @1 spl any :,
fn split(state: &mut State) {
    let split_type = match state.args.get(1) {
        Some(s) => s,
        None => {
            eprintln!("Arg #2 is missing");
            return;
        }
    };
    let tokens = match state.args.get(2) {
        Some(s) => s,
        None => {
            eprintln!("Arg #3 is missing");
            return;
        }
    };

    let Some(list_to_process) = get_arg1_list(state) else {
        return;
    };

    let ListItem::Str(str_to_process) = list_to_process else {
        eprintln!("Expected string, got list");
        return;
    };

    let mut new_list: Vec<ListItem> = Vec::new();
    match split_type.as_str() {
        "seq" => {
            for s in str_to_process.split(tokens) {
                new_list.push(ListItem::Str(s.to_owned()));
            }
        }
        "any" => {
            for s in str_to_process.split(|c| tokens.contains(c)) {
                new_list.push(ListItem::Str(s.to_owned()));
            }
        }
        _ => {
            eprintln!("Unknown split type: {}", split_type);
            return;
        }
    };
    let Some(list_to_change) = get_arg1_list_mut(state) else {
        return;
    };
    *list_to_change = ListItem::List(new_list);
}

// Leave only specific list items
// Example:
// - @1 only
fn only(state: &mut State) {
    leave_only_arg1(state);
}

// Trim strings in list
// Example:
// - @1 trim
fn trim(state: &mut State) {
    let Some(list_to_change) = get_arg1_list_mut(state) else {
        return;
    };
    let ListItem::List(list_to_change) = list_to_change else {
        eprintln!("Expected list, got string");
        return;
    };
    list_to_change.retain_mut(|item| match item {
        ListItem::Str(item_as_str) => {
            let new_str = item_as_str.trim().to_owned();
            if new_str.is_empty() {
                return false;
            } else {
                *item_as_str = new_str;
                return true;
            }
        }
        ListItem::List(_) => return true,
    });
}

fn process_input(input: String, state: &mut State) {
    // convert to state
    let mut iter = input.split(" ");
    let arg1 = match iter.next() {
        Some(s) => s.to_owned(),
        None => {
            eprintln!("Arg #1 is missing");
            return;
        }
    };
    state.args.clear();
    state.args.push(arg1);
    let function_name = match iter.next() {
        Some(s) => s.to_owned(),
        None => {
            eprintln!("Function name is missing");
            return;
        }
    };

    for arg in iter {
        state.args.push(arg.to_owned());
    }

    match function_name.as_str() {
        "print" => print_data_list(state),
        "fread" => file_read(state),
        "spl" => split(state),
        "only" => only(state),
        "trim" => trim(state),
        _ => eprintln!("Unknown function name: {}", function_name),
    };
}

fn main() {
    let mut state = State {
        args: Vec::new(),
        data_list: ListItem::Str(String::new()),
    };

    loop {
        print!("> ");
        let _ = stdout().flush();
        let mut line = String::new();
        stdin().read_line(&mut line).unwrap();
        line = line.trim().to_owned();

        if line == "q" {
            break;
        }

        process_input(line, &mut state);
    }
}
