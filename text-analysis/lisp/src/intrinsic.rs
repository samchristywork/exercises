use crate::Environment;
use crate::Node;
use crate::Range;
use crate::Token;
use crate::TokenKind;
use crate::evaluate_node;
use crate::process_file;

macro_rules! evaluate_args {
    ($args:expr, $env:expr) => {
        $args
            .iter()
            .map(|arg| evaluate_node(arg, $env))
            .collect::<Vec<_>>()
    };
}

macro_rules! symbol {
    ($value:expr) => {
        Node {
            token: Token {
                value: $value.to_string(),
                kind: TokenKind::Symbol,
                range: Range { start: 0, end: 0 },
            },
            children: Vec::new(),
        }
    };
}

macro_rules! expect_number {
    ($arg:expr, $env:expr) => {
        evaluate_node($arg, $env).token.value.parse::<i32>().expect("Invalid number")
    };
}

pub fn fn_add(args: &[Node], env: &mut Environment) -> Node {
    Node {
        token: Token {
            value: evaluate_args!(args, env)
                .iter()
                .map(|arg| arg.token.value.parse::<i32>().expect("Invalid number"))
                .sum::<i32>()
                .to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_sub(args: &[Node], env: &mut Environment) -> Node {
    let mut remaining_args = evaluate_args!(args, env);
    let first_arg = remaining_args
        .remove(0)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

    Node {
        token: Token {
            value: remaining_args
                .into_iter()
                .fold(first_arg, |acc, x| {
                    acc - x.token.value.parse::<i32>().expect("Invalid number")
                })
                .to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_mul(args: &[Node], env: &mut Environment) -> Node {
    Node {
        token: Token {
            value: evaluate_args!(args, env)
                .iter()
                .map(|arg| arg.token.value.parse::<i32>().expect("Invalid number"))
                .product::<i32>()
                .to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_pow(args: &[Node], env: &mut Environment) -> Node {
    let base = expect_number!(&args[0], env);
    let exponent = expect_number!(&args[1], env);

    Node {
        token: Token {
            value: (base.pow(exponent.try_into().expect("Invalid exponent"))).to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_mod(args: &[Node], env: &mut Environment) -> Node {
    let dividend = expect_number!(&args[0], env);
    let divisor = expect_number!(&args[1], env);

    Node {
        token: Token {
            value: (dividend % divisor).to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_inc(args: &[Node], env: &mut Environment) -> Node {
    let n = expect_number!(&args[0], env);
    Node {
        token: Token {
            value: (n + 1).to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_dec(args: &[Node], env: &mut Environment) -> Node {
    let n = expect_number!(&args[0], env);
    Node {
        token: Token {
            value: (n - 1).to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_max(args: &[Node], env: &mut Environment) -> Node {
    let max_value = evaluate_args!(args, env)
        .iter()
        .map(|arg| arg.token.value.parse::<i32>().expect("Invalid number"))
        .max()
        .unwrap_or(0);

    Node {
        token: Token {
            value: max_value.to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_min(args: &[Node], env: &mut Environment) -> Node {
    let min_value = evaluate_args!(args, env)
        .iter()
        .map(|arg| arg.token.value.parse::<i32>().expect("Invalid number"))
        .min()
        .unwrap_or(0);

    Node {
        token: Token {
            value: min_value.to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_is_text(args: &[Node], env: &mut Environment) -> Node {
    let arg = evaluate_node(&args[0], env);
    if arg.token.kind == TokenKind::Text {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_is_number(args: &[Node], env: &mut Environment) -> Node {
    let arg = evaluate_node(&args[0], env);
    if arg.token.kind == TokenKind::Number {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_is_symbol(args: &[Node], env: &mut Environment) -> Node {
    let arg = evaluate_node(&args[0], env);
    if arg.token.kind == TokenKind::Symbol {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_is_lparen(args: &[Node], env: &mut Environment) -> Node {
    let arg = evaluate_node(&args[0], env);
    if arg.token.kind == TokenKind::LParen {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_is_lambda(args: &[Node], env: &mut Environment) -> Node {
    let arg = evaluate_node(&args[0], env);
    if arg.token.kind == TokenKind::Lambda {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_is_atom(args: &[Node], env: &mut Environment) -> Node {
    let arg = evaluate_node(&args[0], env);
    if arg.token.kind == TokenKind::Atom {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_and(args: &[Node], env: &mut Environment) -> Node {
    if args
        .iter()
        .all(|arg| evaluate_node(arg, env).token.value == "true")
    {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_or(args: &[Node], env: &mut Environment) -> Node {
    if args
        .iter()
        .any(|arg| evaluate_node(arg, env).token.value == "true")
    {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_repeat(args: &[Node], env: &mut Environment) -> Node {
    (0..evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number"))
        .for_each(|_| {
            evaluate_node(&args[1], env);
        });

    symbol!("true")
}

pub fn fn_loop(args: &[Node], env: &mut Environment) -> Node {
    loop {
        evaluate_node(&args[0], env);
    }
}

pub fn fn_write(args: &[Node], env: &mut Environment) -> Node {
    println!(
        "{}",
        evaluate_args!(args, env)
            .iter()
            .map(super::Node::string)
            .collect::<Vec<_>>()
            .join(" ")
            .as_str()
    );

    symbol!("true")
}

pub fn fn_write_stderr(args: &[Node], env: &mut Environment) -> Node {
    eprintln!(
        "{}",
        evaluate_args!(args, env)
            .iter()
            .map(super::Node::string)
            .collect::<Vec<_>>()
            .join(" ")
            .as_str()
    );

    symbol!("true")
}

pub fn fn_write_file(args: &[Node], env: &mut Environment) -> Node {
    let filename = evaluate_node(&args[0], env).token.value;
    let content = evaluate_node(&args[1], env).token.value;

    std::fs::write(filename, content).expect("Unable to write file");

    symbol!("true")
}

pub fn fn_read_file(args: &[Node], env: &mut Environment) -> Node {
    let filename = evaluate_node(&args[0], env).token.value;
    let content = std::fs::read_to_string(filename).expect("Unable to read file");

    Node {
        token: Token {
            value: content,
            kind: TokenKind::Text,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_join(args: &[Node], env: &mut Environment) -> Node {
    let separator = evaluate_node(&args[0], env).token.value;
    let strings = evaluate_args!(&args[1].children, env)
        .iter()
        .map(|arg| arg.token.value.clone())
        .collect::<Vec<_>>()
        .join(&separator);

    Node {
        token: Token {
            value: strings,
            kind: TokenKind::Text,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_split(args: &[Node], env: &mut Environment) -> Node {
    let separator = evaluate_node(&args[0], env).token.value;
    let string = evaluate_node(&args[1], env).token.value;

    let parts = string.split(&separator).map(|s| {
        Node {
            token: Token {
                value: s.to_string(),
                kind: TokenKind::Text,
                range: Range { start: 0, end: 0 },
            },
            children: Vec::new(),
        }
    });

    Node {
        token: Token {
            value: String::from("split"),
            kind: TokenKind::LParen,
            range: Range { start: 0, end: 0 },
        },
        children: parts.collect(),
    }
}

pub fn fn_empty_string(args: &[Node], env: &mut Environment) -> Node {
    let arg = evaluate_node(&args[0], env);
    if arg.token.value.is_empty() {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_print_env(args: &[Node], env: &Environment) -> Node {
    if args.is_empty() {
        env.variables.iter().for_each(|(key, value)| {
            println!("{}: {}", key, value.string());
        });
    } else {
        env.variables.iter().for_each(|(key, value)| {
            if key == &args[0].token.value {
                println!("{}: {}", key, value.string());
            }
        });
    }

    symbol!("true")
}

pub fn fn_equal(args: &[Node], env: &mut Environment) -> Node {
    let a = evaluate_node(&args[0], env);
    let b = evaluate_node(&args[1], env);

    if a.token.value == b.token.value {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_if(args: &[Node], env: &mut Environment) -> Node {
    if evaluate_node(&args[0], env).token.value == "true" {
        return evaluate_node(&args[1], env);
    }

    evaluate_node(&args[2], env)
}

pub fn fn_cond(args: &[Node], env: &mut Environment) -> Node {
    args.iter()
        .find_map(|arg| {
            if evaluate_node(&arg.children[0], env).token.value == "true" {
                Some(evaluate_node(&arg.children[1], env))
            } else {
                None
            }
        })
        .unwrap_or_else(|| symbol!("false"))
}

pub fn fn_less_than(args: &[Node], env: &mut Environment) -> Node {
    let a = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");
    let b = evaluate_node(&args[1], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

    if a < b {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_greater_than(args: &[Node], env: &mut Environment) -> Node {
    let a = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");
    let b = evaluate_node(&args[1], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

    if a > b {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_def(args: &[Node], env: &mut Environment) -> Node {
    let name = &args[0].token.value;
    let value = evaluate_node(&args[1], env);
    env.set(name.clone(), value);

    symbol!("true")
}

fn look_for_bang(children: &[Node]) -> bool {
    children.iter().any(|child| {
        if child.token.kind == TokenKind::Symbol {
            child.token.value.ends_with('!')
        } else {
            look_for_bang(&child.children)
        }
    })
}

pub fn fn_func(args: &[Node], env: &mut Environment) -> Node {
    let name = &args[0].token.value;
    let params = args[1].clone();
    let body = args[2..].to_vec();
    let mut children = vec![params];
    children.extend(body);

    let name_ends_with_bang = name.ends_with('!');
    let at_least_one_child_identifier_ends_with_bang = look_for_bang(&children);
    assert_eq!(
        name_ends_with_bang,
        at_least_one_child_identifier_ends_with_bang,
        "Function name and child identifiers must match the bang convention",
    );

    let lambda = Node {
        token: Token {
            value: String::from("lambda"),
            kind: TokenKind::Lambda,
            range: Range { start: 0, end: 0 },
        },
        children,
    };

    env.set(name.clone(), lambda);

    symbol!("true")
}

pub fn fn_read_line() -> Node {
    let mut input = String::new();
    std::io::stdin()
        .read_line(&mut input)
        .expect("Failed to read line");

    Node {
        token: Token {
            value: input.trim().to_string(),
            kind: TokenKind::Text,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_list(args: &[Node], env: &mut Environment) -> Node {
    let list = evaluate_args!(args, env);
    Node {
        token: Token {
            value: String::from("list"),
            kind: TokenKind::LParen,
            range: Range { start: 0, end: 0 },
        },
        children: list,
    }
}

pub fn fn_map(args: &[Node], env: &mut Environment) -> Node {
    let function = &args[0];
    let list = &args[1];

    let children = list
        .children
        .iter()
        .map(|item| Node {
            token: Token {
                value: String::from("map"),
                kind: TokenKind::LParen,
                range: Range { start: 0, end: 0 },
            },
            children: vec![function.clone(), item.clone()],
        })
        .collect::<Vec<_>>()
        .iter()
        .map(|child| evaluate_node(child, env))
        .collect::<Vec<_>>();

    Node {
        token: Token {
            value: String::from("map"),
            kind: TokenKind::LParen,
            range: Range { start: 0, end: 0 },
        },
        children,
    }
}

pub fn fn_filter(args: &[Node], env: &mut Environment) -> Node {
    let function = &args[0];
    let list = &args[1];

    let children = list
        .children
        .iter()
        .filter_map(|item| {
            let result = Node {
                token: Token {
                    value: String::from("filter"),
                    kind: TokenKind::LParen,
                    range: Range { start: 0, end: 0 },
                },
                children: vec![function.clone(), item.clone()],
            };
            if evaluate_node(&result, env).token.value == "true" {
                Some(item.clone())
            } else {
                None
            }
        })
        .collect::<Vec<_>>()
        .iter()
        .map(|child| evaluate_node(child, env))
        .collect::<Vec<_>>();

    Node {
        token: Token {
            value: String::from("filter"),
            kind: TokenKind::LParen,
            range: Range { start: 0, end: 0 },
        },
        children,
    }
}

pub fn fn_true() -> Node {
    symbol!("true")
}

pub fn fn_false() -> Node {
    symbol!("false")
}

pub fn fn_is_even(args: &[Node], env: &mut Environment) -> Node {
    let number = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

    if number % 2 == 0 {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_is_odd(args: &[Node], env: &mut Environment) -> Node {
    let number = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

    if number % 2 != 0 {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_get_environment_variable(args: &[Node], env: &mut Environment) -> Node {
    let var_name = evaluate_node(&args[0], env).token.value;
    let value = std::env::var(var_name).unwrap_or_else(|_| String::from("nil"));
    Node {
        token: Token {
            value,
            kind: TokenKind::Text,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_head(args: &[Node], env: &mut Environment) -> Node {
    let list = evaluate_node(&args[0], env);
    if list.children.is_empty() {
        symbol!("nil")
    } else {
        list.children[0].clone()
    }
}

pub fn fn_last(args: &[Node], env: &mut Environment) -> Node {
    let list = evaluate_node(&args[0], env);
    if list.children.is_empty() {
        symbol!("nil")
    } else {
        list.children.last().expect("Last element not found").clone()
    }
}

pub fn fn_tail(args: &[Node], env: &mut Environment) -> Node {
    let list = evaluate_node(&args[0], env);
    if list.children.is_empty() {
        symbol!("nil")
    } else {
        Node {
            token: Token {
                value: String::from("tail"),
                kind: TokenKind::LParen,
                range: Range { start: 0, end: 0 },
            },
            children: list.children[1..].to_vec(),
        }
    }
}

pub fn fn_length(args: &[Node], env: &mut Environment) -> Node {
    let list = evaluate_node(&args[0], env);
    Node {
        token: Token {
            value: list.children.len().to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_load(args: &[Node], env: &mut Environment) -> Node {
    let filename = evaluate_node(&args[0], env).token.value;
    process_file(&filename, env, true, false, false);
    symbol!("true")
}

pub fn fn_url_encode(args: &[Node], env: &mut Environment) -> Node {
    let input = evaluate_node(&args[0], env).token.value;
    let encoded = url::form_urlencoded::byte_serialize(input.as_bytes()).collect::<String>();

    Node {
        token: Token {
            value: encoded,
            kind: TokenKind::Text,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_url_decode(args: &[Node], env: &mut Environment) -> Node {
    let input = evaluate_node(&args[0], env).token.value;
    let decoded = url::form_urlencoded::parse(input.as_bytes())
        .map(|(key, value)| format!("{key}={value}"))
        .collect::<Vec<_>>()
        .join("&");

    Node {
        token: Token {
            value: decoded,
            kind: TokenKind::Text,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_sleep(args: &[Node], env: &mut Environment) -> Node {
    let duration = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<u64>()
        .expect("Invalid number");
    std::thread::sleep(std::time::Duration::from_secs(duration));
    symbol!("true")
}

pub fn fn_sleep_ms(args: &[Node], env: &mut Environment) -> Node {
    let duration = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<u64>()
        .expect("Invalid number");
    std::thread::sleep(std::time::Duration::from_millis(duration));
    symbol!("true")
}
