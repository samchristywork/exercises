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
        {
            let arg = evaluate_node($arg, $env);
            assert_eq!(
                arg.token.kind,
                TokenKind::Number,
                "Expected a number, but got: {}",
                arg.token.value
            );
            arg.token.value.parse::<i32>().expect("Invalid number")
        }
    };
}

macro_rules! expect_text {
    ($arg:expr, $env:expr) => {
        {
            let arg = evaluate_node($arg, $env);
            assert_eq!(
                arg.token.kind,
                TokenKind::Text,
                "Expected a text, but got: {}",
                arg.token.value
            );
            arg.token.value.clone()
        }
    };
}

macro_rules! expect_symbol {
    ($arg:expr, $env:expr) => {
        {
            let arg = evaluate_node($arg, $env);
            assert_eq!(
                arg.token.kind,
                TokenKind::Symbol,
                "Expected a symbol, but got: {}",
                arg.token.value
            );
            arg
        }
    };
}

macro_rules! expect_list {
    ($arg:expr, $env:expr) => {
        {
            let arg = evaluate_node($arg, $env);
            assert_eq!(
                arg.token.kind,
                TokenKind::LParen,
                "Expected a list, but got: {}",
                arg.token.value
            );
            arg.children.clone()
        }
    };
}

macro_rules! expect_n_args {
    ($args:expr, $n:expr) => {
        if $args.len() != $n {
            panic!("Expected {} arguments, but got {}", $n, $args.len());
        }
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
    expect_n_args!(args, 2);

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
    expect_n_args!(args, 2);

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
    expect_n_args!(args, 1);

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
    expect_n_args!(args, 1);

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

macro_rules! is_type {
    ($arg:expr, $env:expr, $kind:expr) => {
        if evaluate_node($arg, $env).token.kind == $kind {
            symbol!("true")
        } else {
            symbol!("false")
        }
    };
}

pub fn fn_is_text(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    is_type!(&args[0], env, TokenKind::Text)
}

pub fn fn_is_number(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    is_type!(&args[0], env, TokenKind::Number)
}

pub fn fn_is_symbol(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    is_type!(&args[0], env, TokenKind::Symbol)
}

pub fn fn_is_lparen(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    is_type!(&args[0], env, TokenKind::LParen)
}

pub fn fn_is_lambda(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    is_type!(&args[0], env, TokenKind::Lambda)
}

pub fn fn_is_atom(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    is_type!(&args[0], env, TokenKind::Atom)
}

pub fn fn_and(args: &[Node], env: &mut Environment) -> Node {
    if args
        .iter()
        .all(|arg| expect_symbol!(&arg, env).token.value == "true")
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
    expect_n_args!(args, 2);

    let n = expect_number!(&args[0], env);
    (0..n).fold(symbol!("false"), |_, _| {
        evaluate_node(&args[1], env)
    })
}

pub fn fn_loop(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

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
    expect_n_args!(args, 2);

    let filename = expect_text!(&args[0], env);
    let content = expect_text!(&args[1], env);

    std::fs::write(filename, content).expect("Unable to write file");

    symbol!("true")
}

pub fn fn_read_file(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let filename = expect_text!(&args[0], env);
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
    expect_n_args!(args, 2);

    let separator = expect_text!(&args[0], env);
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
    expect_n_args!(args, 2);

    let separator = expect_text!(&args[0], env);
    let string = expect_text!(&args[1], env);

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

pub fn fn_lines(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let string = expect_text!(&args[0], env);
    let lines = string.lines().map(|s| {
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
            value: String::from("lines"),
            kind: TokenKind::LParen,
            range: Range { start: 0, end: 0 },
        },
        children: lines.collect(),
    }
}

pub fn fn_strlen(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let string = expect_text!(&args[0], env);
    Node {
        token: Token {
            value: string.len().to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_empty_string(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let arg = expect_text!(&args[0], env);
    if arg.is_empty() {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_print_env(args: &[Node], env: &mut Environment) -> Node {
    let red = "\x1b[31m";
    let normal = "\x1b[0m";
    print!("{red}");
    env.variables.iter().for_each(|(key, value)| {
        println!("{}: {}", key, value.string());
    });
    println!("{normal}");

    symbol!("true")
}

pub fn test_equal(a: &Node, b: &Node) -> bool {
    if a.token.kind != b.token.kind {
        return false;
    }

    if a.token.kind == TokenKind::LParen {
        if a.children.len() != b.children.len() {
            return false;
        }
        for (a_child, b_child) in a.children.iter().zip(&b.children) {
            if !test_equal(a_child, b_child) {
                return false;
            }
        }
    } else if a.token.value != b.token.value {
        return false;
    }

    true
}

pub fn fn_equal(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    let a = evaluate_node(&args[0], env);
    let b = evaluate_node(&args[1], env);

    if test_equal(&a, &b) {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_if(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 3);

    if expect_symbol!(&args[0], env).token.value == "true" {
        return evaluate_node(&args[1], env);
    }

    evaluate_node(&args[2], env)
}

pub fn fn_cond(args: &[Node], env: &mut Environment) -> Node {
    args.iter()
        .find_map(|arg| {
            if expect_symbol!(&arg.children[0], env).token.value == "true" {
                Some(evaluate_node(&arg.children[1], env))
            } else {
                None
            }
        })
        .unwrap_or_else(|| symbol!("false"))
}

pub fn fn_less_than(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if expect_number!(&args[0], env) < expect_number!(&args[1], env) {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_greater_than(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if expect_number!(&args[0], env) > expect_number!(&args[1], env) {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_def(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

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

    assert_eq!(
        name.ends_with('!'),
        look_for_bang(&children),
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

pub fn fn_read_line(args: &[Node]) -> Node {
    expect_n_args!(args, 0);

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
    expect_n_args!(args, 2);

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

pub fn fn_true(args: &[Node]) -> Node {
    expect_n_args!(args, 0);
    symbol!("true")
}

pub fn fn_false(args: &[Node]) -> Node {
    expect_n_args!(args, 0);
    symbol!("false")
}

pub fn fn_is_even(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if expect_number!(&args[0], env) % 2 == 0 {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_is_odd(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if expect_number!(&args[0], env) % 2 != 0 {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_get_environment_variable(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let var_name = expect_text!(&args[0], env);
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
    expect_n_args!(args, 1);

    let list = expect_list!(&args[0], env);
    if list.is_empty() {
        panic!("Empty list");
    } else {
        list[0].clone()
    }
}

pub fn fn_last(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let list = expect_list!(&args[0], env);
    if list.is_empty() {
        panic!("Empty list");
    } else {
        list[list.len() - 1].clone()
    }
}

pub fn fn_tail(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let list = expect_list!(&args[0], env);
    if list.is_empty() {
        panic!("Empty list");
    } else {
        Node {
            token: Token {
                value: String::from("tail"),
                kind: TokenKind::LParen,
                range: Range { start: 0, end: 0 },
            },
            children: list[1..].to_vec(),
        }
    }
}

pub fn fn_length(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let list = expect_list!(&args[0], env);
    Node {
        token: Token {
            value: list.len().to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_load(args: &[Node], env: &mut Environment) -> Node {
    let filename = expect_text!(&args[0], env);
    process_file(&filename, env, true, false, false);
    symbol!("true")
}

pub fn fn_url_encode(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let input = expect_text!(&args[0], env);
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
    expect_n_args!(args, 1);

    let input = expect_text!(&args[0], env);
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
    expect_n_args!(args, 1);

    let duration = expect_number!(&args[0], env);
    std::thread::sleep(std::time::Duration::from_secs(duration.try_into().expect("Invalid duration")));
    symbol!("true")
}

pub fn fn_sleep_ms(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let duration = expect_number!(&args[0], env);
    std::thread::sleep(std::time::Duration::from_millis(duration.try_into().expect("Invalid duration")));
    symbol!("true")
}
