use crate::Environment;
use crate::Node;
use crate::Range;
use crate::Token;
use crate::TokenKind;

macro_rules! evaluate_args {
    ($args:expr, $env:expr) => {
        $args.iter()
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

fn fn_add(args: &[Node], env: &mut Environment) -> Node {
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

fn fn_sub(args: &[Node], env: &mut Environment) -> Node {
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

fn fn_mul(args: &[Node], env: &mut Environment) -> Node {
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

fn fn_repeat(args: &[Node], env: &mut Environment) -> Node {
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

fn fn_loop(args: &[Node], env: &mut Environment) -> Node {
    loop {
        evaluate_node(&args[0], env);
    }
}

fn fn_print(args: &[Node], env: &mut Environment) -> Node {
    println!(
        "{}",
        evaluate_args!(args, env)
            .iter()
            .map(|arg| arg.string())
            .collect::<Vec<_>>()
            .join(" ")
            .as_str()
    );

    symbol!("true")
}

fn fn_join(args: &[Node], env: &mut Environment) -> Node {
    let separator = evaluate_node(&args[0], env).token.value.clone();
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

fn fn_printenv(args: &[Node], env: &mut Environment) -> Node {
    if args.len() == 0 {
        env.variables.iter().for_each(|(key, value)| {
            println!("{}: {}", key, value);
        });
    } else {
        env.variables.iter().for_each(|(key, value)| {
            if key == &args[0].token.value {
                println!("{}: {}", key, value);
            }
        });
    }

    symbol!("true")
}

fn fn_equal(args: &[Node], env: &mut Environment) -> Node {
    let a = evaluate_node(&args[0], env);
    let b = evaluate_node(&args[1], env);

    if a.token.value == b.token.value {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

fn fn_if(args: &[Node], env: &mut Environment) -> Node {
    if evaluate_node(&args[0], env).token.value == "true" {
        return evaluate_node(&args[1], env);
    }

    evaluate_node(&args[2], env)
}

fn fn_cond(args: &[Node], env: &mut Environment) -> Node {
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

fn fn_less_than(args: &[Node], env: &mut Environment) -> Node {
    if evaluate_node(&args[0], env).token.value < evaluate_node(&args[1], env).token.value {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

fn fn_greater_than(args: &[Node], env: &mut Environment) -> Node {
    if evaluate_node(&args[0], env).token.value > evaluate_node(&args[1], env).token.value {
        symbol!("true")
    } else {
        symbol!("false")
    }
}

fn fn_def(args: &[Node], env: &mut Environment) -> Node {
    let name = &args[0].token.value;
    let value = evaluate_node(&args[1], env);
    env.set(name.clone(), value);

    symbol!("true")
}

fn fn_defun(args: &[Node], env: &mut Environment) -> Node {
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

fn fn_read_line() -> Node {
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

fn fn_list(args: &[Node], env: &mut Environment) -> Node {
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

fn fn_map(args: &[Node], env: &mut Environment) -> Node {
    let function = &args[0];
    let list = &args[1];

    let children = list
        .children
        .iter()
        .map(|item| {
            Node {
                token: Token {
                    value: String::from("map"),
                    kind: TokenKind::LParen,
                    range: Range { start: 0, end: 0 },
                },
                children: vec![function.clone(), item.clone()],
            }
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

fn apply_function(function: &Node, args: &[Node], env: &mut Environment) -> Node {
    match function.token.kind {
        TokenKind::Symbol => match function.token.value.as_str() {
            "+" => fn_add(args, env),
            "-" => fn_sub(args, env),
            "*" => fn_mul(args, env),
            "repeat" => fn_repeat(args, env),
            "loop" => fn_loop(args, env),
            "print" => fn_print(args, env),
            "join" => fn_join(args, env),
            "printenv" => fn_printenv(args, env),
            "true" => symbol!("true"),
            "false" => symbol!("false"),
            "=" => fn_equal(args, env),
            "if" => fn_if(args, env),
            "cond" => fn_cond(args, env),
            "<" => fn_less_than(args, env),
            ">" => fn_greater_than(args, env),
            "def" => fn_def(args, env),
            "defun" => fn_defun(args, env),
            "read-line" => fn_read_line(),
            "list" => fn_list(args, env),
            "map" => fn_map(args, env),
            _ => env.get(&function.token.value).map_or_else(
                || panic!("Unknown function: {}", function.token.value),
                std::clone::Clone::clone,
            ),
        },
        TokenKind::Lambda => {
            let params = &function.children[0];
            let body = &function.children[1..];

            if params.children.len() != args.len() {
                panic!("Argument count mismatch");
            }

            let mut new_env = env.clone();
            for (param, arg) in params.children.iter().zip(args) {
                new_env.set(param.token.value.clone(), evaluate_node(arg, env));
            }

            let mut return_value = symbol!("true");
            for child in body {
                return_value = evaluate_node(child, &mut new_env);
            }

            return_value
        }
        TokenKind::LParen => {
            apply_function(&function.children[0].clone(), &function.children[1..], env)
        }
        _ => panic!("Invalid function application"),
    }
}

pub fn evaluate_node(node: &Node, env: &mut Environment) -> Node {
    match node.token.kind {
        TokenKind::Module => {
            for child in &node.children {
                evaluate_node(child, env);
            }

            Node {
                token: node.token.clone(),
                children: Vec::new(),
            }
        }
        TokenKind::LParen => {
            let function = evaluate_node(&node.children[0], env);
            apply_function(&function, &node.children[1..], env)
        }
        TokenKind::Symbol => env
            .get(&node.token.value)
            .map_or_else(|| node.clone(), std::clone::Clone::clone),
        _ => node.clone(),
    }
}
