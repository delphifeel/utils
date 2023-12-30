use std::fs::File;
use std::io::{stdin, stdout, Read, Write};

#[derive(Debug)]
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

enum Indexes {
    List(Vec<usize>),
    All,
}

// Arg 1 possible values:
// - @0 - mean whole data list
// - @N - Nth list
// - @N{a,b,c} - list subset of a, b, c strings
fn get_arg1_list(state: &State) -> Option<(&ListItem, Indexes)> {
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

            let mut iter = s.split("{");
            let Some(number_str) = iter.next() else {
                eprintln!("Wrong arg format");
                return None;
            };
            let i: usize = number_str.parse().unwrap();

            let list_to_return = match i {
                0 => &state.data_list,
                _ => {
                    let ListItem::List(ref ls) = state.data_list else {
                        eprintln!("Expected list, got string");
                        return None;
                    };
                    let Some(l) = ls.get(i - 1) else {
                        eprintln!("List #{} is missing", i);
                        return None;
                    };
                    l
                }
            };
            let Some(keys) = iter.next() else {
                return Some((list_to_return, Indexes::All));
            };
            // if @N{a,b,c}
            let iter = keys.split(|c| c == ',' || c == '}');
            let mut keys_to_test: Vec<&str> = iter.collect();
            keys_to_test.remove(keys_to_test.len() - 1);
            let ListItem::List(ls) = list_to_return else {
                eprintln!("Expect list, got string");
                return None;
            };
            let indexes: Vec<usize> = ls
                .iter()
                .enumerate()
                .filter_map(|(i, item)| {
                    let ListItem::Str(item_str) = item else {
                        return None;
                    };
                    return match keys_to_test.contains(&item_str.as_str()) {
                        true => Some(i),
                        false => None,
                    };
                })
                .collect();
            return Some((list_to_return, Indexes::List(indexes)));
        }
        _ => {
            eprintln!("Wrong format for Arg #1: @N expected");
            return None;
        }
    };
}

fn get_arg1_list_mut(state: &mut State) -> Option<(&mut ListItem, Indexes)> {
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

            let mut iter = s.split("{");
            let Some(number_str) = iter.next() else {
                eprintln!("Wrong arg format");
                return None;
            };
            let i: usize = number_str.parse().unwrap();

            let list_to_return = match i {
                0 => &mut state.data_list,
                _ => {
                    let ListItem::List(ref mut ls) = state.data_list else {
                        eprintln!("Expected list, got string");
                        return None;
                    };
                    let Some(l) = ls.get_mut(i - 1) else {
                        eprintln!("List #{} is missing", i);
                        return None;
                    };
                    l
                }
            };
            let Some(keys) = iter.next() else {
                return Some((list_to_return, Indexes::All));
            };
            // if @N{a,b,c}
            let iter = keys.split(|c| c == ',' || c == '}');
            let mut keys_to_test: Vec<&str> = iter.collect();
            keys_to_test.remove(keys_to_test.len() - 1);
            let ListItem::List(ls) = list_to_return else {
                eprintln!("Expect list, got string");
                return None;
            };
            let indexes: Vec<usize> = ls
                .iter()
                .enumerate()
                .filter_map(|(i, item)| {
                    let ListItem::Str(item_str) = item else {
                        return None;
                    };
                    return match keys_to_test.contains(&item_str.as_str()) {
                        true => Some(i),
                        false => None,
                    };
                })
                .collect();
            return Some((list_to_return, Indexes::List(indexes)));
        }
        _ => {
            eprintln!("Wrong format for Arg #1: @N expected");
            return None;
        }
    };
}

fn leave_only_arg1(state: &mut State) {
    let Some(arg1) = state.args.get(0) else {
        eprintln!("Arg #1 is missing");
        return;
    };

    match arg1.get(0..1) {
        Some("@") => {
            let Some(s) = arg1.get(1..) else {
                eprintln!("Wrong format for Arg #1: @N expected");
                return;
            };

            let mut iter = s.split("{");
            let Some(number_str) = iter.next() else {
                eprintln!("Wrong arg format");
                return;
            };
            let i: usize = number_str.parse().unwrap();
            state.data_list = match i {
                0 => return,
                _ => {
                    let ListItem::List(ref mut ls) = state.data_list else {
                        eprintln!("Expected data list to be list, not string");
                        return;
                    };
                    let n = i - 1;
                    if n >= ls.len() {
                        eprintln!("List #{} is missing", i);
                        return;
                    }
                    let l = ls.swap_remove(i - 1);
                    l
                }
            };
            let Some(keys) = iter.next() else {
                return;
            };
            // if @N{a,b,c}
            let iter = keys.split(|c| c == ',' || c == '}');
            let mut keys_to_test: Vec<&str> = iter.collect();
            keys_to_test.remove(keys_to_test.len() - 1);
            let ListItem::List(ref mut ls) = state.data_list else {
                eprintln!("Expect list, got string");
                return;
            };
            ls.retain(|item| {
                let ListItem::Str(item_str) = item else {
                    return true;
                };
                return keys_to_test.contains(&item_str.as_str());
            });
        }
        _ => {
            eprintln!("Wrong format for Arg #1: @N expected");
            return;
        }
    };
}
const DEFAULT_MAX_PRINT_STR_SIZE: usize = 30;

fn print_list_item(
    level: u32,
    list_item: &ListItem,
    indexes: &Indexes,
    max_str_size: Option<usize>,
) {
    match list_item {
        ListItem::Str(ref s) => match max_str_size {
            None => print!("\"{}\", ", s),
            Some(max_size_v) if max_size_v > s.len() => print!("\"{}\", ", s),
            _ => print!("\"{}...\", ", &s[0..DEFAULT_MAX_PRINT_STR_SIZE]),
        },
        ListItem::List(ref ls) => {
            print!("[ ");

            match (level, indexes) {
                (0, Indexes::List(indexes_list)) => {
                    for i in indexes_list {
                        let l = ls.get(*i).unwrap();
                        print_list_item(level + 1, l, indexes, max_str_size);
                    }
                }
                _ => {
                    for l in ls {
                        print_list_item(level + 1, l, indexes, max_str_size);
                    }
                }
            }

            print!("]");
        }
    }
}

// @1 print full
fn print_data_list(state: &mut State) {
    let Some((list_item, indexes)) = get_arg1_list(state) else {
        return;
    };

    if state.args.get(1).is_some_and(|s| s == "full") {
        print_list_item(0, list_item, &indexes, None);
        println!();
    } else {
        print_list_item(0, list_item, &indexes, Some(DEFAULT_MAX_PRINT_STR_SIZE));
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

    let Some((item_to_process, _)) = get_arg1_list(state) else {
        return;
    };

    let ListItem::Str(str_to_process) = item_to_process else {
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
    let Some((item_to_process, _)) = get_arg1_list_mut(state) else {
        return;
    };
    *item_to_process = ListItem::List(new_list);
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
// - @1{a,b,c} trim
fn trim(state: &mut State) {
    let Some((list_item, indexes)) = get_arg1_list_mut(state) else {
        return;
    };

    let ListItem::List(list_to_change) = list_item else {
        eprintln!("Expected list, got string");
        return;
    };
    let mut i = 0;
    list_to_change.retain_mut(|item| {
        if let Indexes::List(ref indexes_list) = indexes {
            if !indexes_list.contains(&i) {
                return true;
            }
        }
        i += 1;
        match item {
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
        }
    });
}

// Take items and their next items
// Example:
// - @1{target,charges,maxCharges} and_next_only
fn and_next_only(state: &mut State) {
    let Some((list, indexes)) = get_arg1_list_mut(state) else {
        return;
    };
    let ListItem::List(ls) = list else {
        eprintln!("Expect list, got string");
        return;
    };
    let Indexes::List(indexes_list) = indexes else {
        eprintln!("Expected pattern matching, got whole list");
        return;
    };
    let mut new_indexes: Vec<usize> = Vec::new();
    for i in indexes_list.iter() {
        new_indexes.push(*i);
        if *i < ls.len() - 1 {
            new_indexes.push(*i + 1);
        }
    }
    let mut i = 0;
    ls.retain(|_| {
        let v = new_indexes.contains(&i);
        i += 1;
        return v;
    });
}

// Converts list to json and set it to data list
// Example:
// - @1 to_json
fn to_json(state: &mut State) {
    let Some((list, indexes)) = get_arg1_list(state) else {
        return;
    };
    if let Indexes::List(_) = indexes {
        eprintln!("Expected whole list, got matching");
        return;
    }
    let ListItem::List(ls) = list else {
        eprintln!("Expect list, got string");
        return;
    };
    let mut i = 0;
    let mut json_str = "{\n".to_owned();
    for item in ls {
        let ListItem::Str(s) = item else {
            eprintln!("Expect string, got list");
            return;
        };
        if i % 2 == 0 {
            json_str += format!("  \"{s}\": ").as_str();
        } else {
            let comma = if i < ls.len() - 1 { "," } else { "" };
            match s.parse::<u32>() {
                Ok(n) => json_str += format!("{n}{comma}\n").as_str(),
                Err(_) => json_str += format!("\"{s}\"{comma}\n").as_str(),
            }
        }
        i += 1;
    }
    json_str += "}";

    state.data_list = ListItem::Str(json_str);
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
        "and_next_only" => and_next_only(state),
        "to_json" => to_json(state),
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

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn process_file_of_cmds() {
        let mut state = State {
            args: Vec::new(),
            data_list: ListItem::Str(String::new()),
        };

        for line in read_file("tests/cmds_1.txt").unwrap().lines() {
            process_input(line.to_owned(), &mut state);
        }

        let expected = read_file("tests/out_1.txt").unwrap();
        let ListItem::Str(s) = state.data_list else {
            panic!("Expected string");
        };
        assert_eq!(expected.trim(), s.trim());
    }
}
