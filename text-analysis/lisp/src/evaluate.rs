use crate::Environment;
use crate::Node;
use crate::Range;
use crate::Token;
use crate::TokenKind;

fn evaluate_args(args: &[Node], env: &mut Environment) -> Vec<Node> {
    args.iter()
        .map(|arg| evaluate_node(arg, env))
        .collect::<Vec<_>>()
}

fn fn_add(args: &[Node], env: &mut Environment) -> Node {
    Node {
        token: Token {
            value: evaluate_args(args, env)
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

fn fn_repeat(args: &[Node], env: &mut Environment) -> Node {
    (0..evaluate_node(&args[0], env)
        .token
        .value
        .parse::<i32>()
        .expect("Invalid number"))
        .for_each(|_| {
            evaluate_node(&args[1], env);
        });

    fn_true()
}

fn fn_loop(args: &[Node], env: &mut Environment) -> Node {
    loop {
        evaluate_node(&args[0], env);
    }
}

fn fn_print(args: &[Node], env: &mut Environment) -> Node {
    evaluate_args(args, env)
        .iter()
        .for_each(|arg| println!("{}", arg.token.value));

    fn_true()
}

fn fn_printenv(args: &[Node], env: &mut Environment) -> Node {
    env.variables.iter().for_each(|(key, value)| {
        println!("{}: {}", key, value.token.value);
    });

    fn_true()
}

fn fn_true() -> Node {
    Node {
        token: Token {
            value: String::from("true"),
            kind: TokenKind::Symbol,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

fn fn_false() -> Node {
    Node {
        token: Token {
            value: String::from("false"),
            kind: TokenKind::Symbol,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    }
}

fn fn_equal(args: &[Node], env: &mut Environment) -> Node {
    let a = evaluate_node(&args[0], env);
    let b = evaluate_node(&args[1], env);

    if a.token.value == b.token.value {
        fn_true()
    } else {
        fn_false()
    }
}

fn fn_if(args: &[Node], env: &mut Environment) -> Node {
    if evaluate_node(&args[0], env).token.value == "true" {
        return evaluate_node(&args[1], env);
    }

    evaluate_node(&args[2], env)
}

fn fn_cond(args: &[Node], env: &mut Environment) -> Node {
    args.iter().find_map(|arg| {
        if evaluate_node(&arg.children[0], env).token.value == "true" {
            Some(evaluate_node(&arg.children[1], env))
        } else {
            None
        }
    }).unwrap_or_else(|| fn_false())
}

fn fn_less_than(args: &[Node], env: &mut Environment) -> Node {
    if evaluate_node(&args[0], env).token.value < evaluate_node(&args[1], env).token.value {
        fn_true()
    } else {
        fn_false()
    }
}

fn fn_greater_than(args: &[Node], env: &mut Environment) -> Node {
    if evaluate_node(&args[0], env).token.value > evaluate_node(&args[1], env).token.value {
        fn_true()
    } else {
        fn_false()
    }
}

fn fn_def(args: &[Node], env: &mut Environment) -> Node {
    let name = &args[0].token.value;
    let value = evaluate_node(&args[1], env);
    env.set(name.clone(), value);

    fn_true()
}

fn fn_defun(args: &[Node], env: &mut Environment) -> Node {
    let name = &args[0].token.value;
    let params = args[1].clone();
    let body = args[2].clone();
    env.set(name.clone(), body);

    fn_true()
}

fn apply_function(function: &Node, args: &[Node], env: &mut Environment) -> Node {
    match function.token.kind {
        TokenKind::Symbol => match function.token.value.as_str() {
            "+" => fn_add(args, env),
            "repeat" => fn_repeat(args, env),
            "loop" => fn_loop(args, env),
            "print" => fn_print(args, env),
            "printenv" => fn_printenv(args, env),
            "true" => fn_true(),
            "false" => fn_false(),
            "equal" => fn_equal(args, env),
            "if" => fn_if(args, env),
            "cond" => fn_cond(args, env),
            "<" => fn_less_than(args, env),
            ">" => fn_greater_than(args, env),
            "def" => fn_def(args, env),
            "defun" => fn_defun(args, env),
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
