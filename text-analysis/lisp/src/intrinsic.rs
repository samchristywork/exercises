use crate::Environment;
use crate::Node;
use crate::Range;
use crate::Token;
use crate::TokenKind;
use crate::evaluate_node;

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
    let base = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");
    let exponent = evaluate_node(&args[1], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

    Node {
        token: Token {
            value: (base.pow(exponent as u32)).to_string(),
            kind: TokenKind::Number,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

pub fn fn_mod(args: &[Node], env: &mut Environment) -> Node {
    let dividend = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");
    let divisor = evaluate_node(&args[1], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

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
    let n = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

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
    let n = evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number");

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

pub fn fn_print(args: &[Node], env: &mut Environment) -> Node {
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

pub fn fn_join(args: &[Node], env: &mut Environment) -> Node {
    let separator = evaluate_node(&args[0], env).token.value;
    let strings = evaluate_args!(&args[1..], env)
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
    if evaluate_node(&args[0], env).token.value < evaluate_node(&args[1], env).token.value {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

pub fn fn_greater_than(args: &[Node], env: &mut Environment) -> Node {
    if evaluate_node(&args[0], env).token.value > evaluate_node(&args[1], env).token.value {
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

pub fn fn_defun(args: &[Node], env: &mut Environment) -> Node {
    let name = &args[0].token.value;
    let params = args[1].clone();
    let body = args[2..].to_vec();
    let mut children = vec![params];
    children.extend(body);

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

pub fn fn_true() -> Node {
    symbol!("true")
}

pub fn fn_false() -> Node {
    symbol!("false")
}
