use crate::Environment;
use crate::Node;
use crate::Range;
use crate::Token;
use crate::TokenKind;

fn apply_function(function: &Node, args: &[Node], env: &mut Environment) -> Node {
    match function.token.kind {
        TokenKind::Symbol => match function.token.value.as_str() {
            "+" => {
                let args = args
                    .iter()
                    .map(|arg| evaluate_node(arg, env))
                    .collect::<Vec<_>>();

                let mut sum = 0;
                for arg in args {
                    if let Ok(num) = arg.token.value.parse::<i32>() {
                        sum += num;
                    } else {
                        panic!("Invalid argument for add: {}", arg.token.value);
                    }
                }
                Node {
                    token: Token {
                        value: sum.to_string(),
                        kind: TokenKind::Number,
                        range: Range { start: 0, end: 0 },
                    },
                    children: Vec::new(),
                }
            }
            "repeat" => {
                let n = evaluate_node(&args[0], env);
                for _ in 0..n.token.value.parse::<i32>().expect("Invalid number") {
                    for arg in &args[1..] {
                        evaluate_node(arg, env);
                    }
                }

                Node {
                    token: Token {
                        value: String::from("repeat"),
                        kind: TokenKind::Symbol,
                        range: Range { start: 0, end: 0 },
                    },
                    children: Vec::new(),
                }
            }
            "loop" => loop {
                evaluate_node(&args[0], env);
            },
            "print" => {
                let args = args
                    .iter()
                    .map(|arg| evaluate_node(arg, env))
                    .collect::<Vec<_>>();

                for arg in args {
                    println!("{}", arg.token.value);
                }

                Node {
                    token: Token {
                        value: String::from("print"),
                        kind: TokenKind::Symbol,
                        range: Range { start: 0, end: 0 },
                    },
                    children: Vec::new(),
                }
            }
            "true" => Node {
                token: Token {
                    value: String::from("true"),
                    kind: TokenKind::Symbol,
                    range: Range { start: 0, end: 0 },
                },
                children: Vec::new(),
            },
            "false" => Node {
                token: Token {
                    value: String::from("false"),
                    kind: TokenKind::Symbol,
                    range: Range { start: 0, end: 0 },
                },
                children: Vec::new(),
            },
            "equal" => {
                let args = args
                    .iter()
                    .map(|arg| evaluate_node(arg, env))
                    .collect::<Vec<_>>();

                assert!(
                    args.len() == 2,
                    "equal function requires exactly 2 arguments"
                );

                let result = if args[0].token.value == args[1].token.value {
                    "true"
                } else {
                    "false"
                };

                Node {
                    token: Token {
                        value: String::from(result),
                        kind: TokenKind::Symbol,
                        range: Range { start: 0, end: 0 },
                    },
                    children: Vec::new(),
                }
            }
            "def" => {
                let name = &args[0].token.value;
                let value = evaluate_node(&args[1], env);
                env.set(name.clone(), value);

                Node {
                    token: Token {
                        value: String::from("def"),
                        kind: TokenKind::Symbol,
                        range: Range { start: 0, end: 0 },
                    },
                    children: Vec::new(),
                }
            }
            "defun" => {
                let name = &args[0].token.value;
                let params = args[1].clone();
                let body = args[2].clone();

                env.set(name.clone(), body);

                Node {
                    token: Token {
                        value: String::from("defun"),
                        kind: TokenKind::Symbol,
                        range: Range { start: 0, end: 0 },
                    },
                    children: Vec::new(),
                }
            }
            "printenv" => {
                for (key, value) in &env.variables {
                    println!("{}: {}", key, value);
                }

                Node {
                    token: Token {
                        value: String::from("printenv"),
                        kind: TokenKind::Symbol,
                        range: Range { start: 0, end: 0 },
                    },
                    children: Vec::new(),
                }
            }
            _ => env.get(&function.token.value).map_or_else(
                || panic!("Unknown function: {}", function.token.value),
                std::clone::Clone::clone,
            ),
        },
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
