use std::io::Write;

use crate::Environment;
use crate::Node;
use crate::Range;
use crate::Token;
use crate::TokenKind;
use crate::Value;
use crate::evaluate_node;
use crate::handle_symbol;
use crate::process_file;

macro_rules! evaluate_args {
    ($args:expr, $env:expr) => {
        $args
            .iter()
            .map(|arg| evaluate_node(arg, $env))
            .collect::<Vec<_>>()
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
    let mut n = 0;

    for arg in evaluate_args!(args, env) {
        if let Value::Number(value) = arg.value {
            n += value;
        } else {
            panic!("Invalid argument for addition function");
        }
    }

    Node {
        token: args[0].token.clone(),
        value: Value::Number(n),
        children: Vec::new(),
    }
}

pub fn fn_sub(args: &[Node], env: &mut Environment) -> Node {
    if let Value::Number(first_arg) = evaluate_node(&args[0], env).value {
        let mut n = first_arg;

        for arg in args.iter().skip(1) {
            if let Value::Number(value) = evaluate_node(arg, env).value {
                n -= value;
            } else {
                panic!("Invalid argument for subtraction function");
            }
        }

        return Node {
            token: args[0].token.clone(),
            value: Value::Number(n),
            children: Vec::new(),
        };
    }

    panic!("Invalid arguments for subtraction function");
}

pub fn fn_mul(args: &[Node], env: &mut Environment) -> Node {
    let mut n = 1;

    for arg in evaluate_args!(args, env) {
        if let Value::Number(value) = arg.value {
            n *= value;
        } else {
            panic!("Invalid argument for multiplication function");
        }
    }

    Node {
        token: args[0].token.clone(),
        value: Value::Number(n),
        children: Vec::new(),
    }
}

pub fn fn_pow(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Number(base) = evaluate_node(&args[0], env).value {
        if let Value::Number(exponent) = evaluate_node(&args[1], env).value {
            return Node {
                token: args[0].token.clone(),
                value: Value::Number(base.pow(exponent.try_into().expect("Invalid exponent"))),
                children: Vec::new(),
            };
        }
    }

    panic!("Invalid arguments for power function");
}

pub fn fn_mod(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Number(dividend) = evaluate_node(&args[0], env).value {
        if let Value::Number(divisor) = evaluate_node(&args[1], env).value {
            return Node {
                token: args[0].token.clone(),
                value: Value::Number(dividend % divisor),
                children: Vec::new(),
            };
        }
    }

    panic!("Invalid arguments for modulo function");
}

pub fn fn_inc(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Number(n) = evaluate_node(&args[0], env).value {
        return Node {
            token: args[0].token.clone(),
            value: Value::Number(n + 1),
            children: Vec::new(),
        };
    }

    panic!("Invalid arguments for increment function");
}

pub fn fn_dec(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Number(n) = evaluate_node(&args[0], env).value {
        return Node {
            token: args[0].token.clone(),
            value: Value::Number(n - 1),
            children: Vec::new(),
        };
    }

    panic!("Invalid arguments for decrement function");
}

pub fn fn_max(args: &[Node], env: &mut Environment) -> Node {
    if args.is_empty() {
        panic!("No arguments provided for max function");
    }

    let max_value = evaluate_args!(args, env)
        .iter()
        .map(|arg| {
            if let Value::Number(value) = arg.value {
                value
            } else {
                panic!("Invalid argument for max function");
            }
        })
        .max()
        .unwrap_or(0);

    Node {
        token: args[0].token.clone(),
        value: Value::Number(max_value),
        children: Vec::new(),
    }
}

pub fn fn_min(args: &[Node], env: &mut Environment) -> Node {
    if args.is_empty() {
        panic!("No arguments provided for min function");
    }

    let min_value = evaluate_args!(args, env)
        .iter()
        .map(|arg| {
            if let Value::Number(value) = arg.value {
                value
            } else {
                panic!("Invalid argument for min function");
            }
        })
        .min()
        .unwrap_or(0);

    Node {
        token: args[0].token.clone(),
        value: Value::Number(min_value),
        children: Vec::new(),
    }
}

pub fn fn_is_text(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    if let Value::Text(_) = evaluate_node(&args[0], env).value {
        return fn_true(&[]);
    } else {
        return fn_false(&[]);
    }
}

pub fn fn_is_number(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    if let Value::Number(_) = evaluate_node(&args[0], env).value {
        return fn_true(&[]);
    } else {
        return fn_false(&[]);
    }
}

pub fn fn_is_symbol(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    if let Value::Symbol(_) = evaluate_node(&args[0], env).value {
        return fn_true(&[]);
    } else {
        return fn_false(&[]);
    }
}

pub fn fn_is_lparen(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    if let Value::LParen() = evaluate_node(&args[0], env).value {
        return fn_true(&[]);
    } else {
        return fn_false(&[]);
    }
}

pub fn fn_is_lambda(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    if let Value::Lambda() = evaluate_node(&args[0], env).value {
        return fn_true(&[]);
    } else {
        return fn_false(&[]);
    }
}

pub fn fn_is_atom(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);
    if let Value::Atom(_) = evaluate_node(&args[0], env).value {
        return fn_true(&[]);
    } else {
        return fn_false(&[]);
    }
}

pub fn fn_and(args: &[Node], env: &mut Environment) -> Node {
    if args.iter().all(|arg| {
        if let Value::Symbol(ref s) = evaluate_node(arg, env).value {
            s == "true"
        } else {
            panic!("Invalid argument for and function");
        }
    }) {
        fn_true(&[])
    } else {
        fn_false(&[])
    }
}

pub fn fn_or(args: &[Node], env: &mut Environment) -> Node {
    if args.iter().any(|arg| {
        if let Value::Symbol(ref s) = evaluate_node(arg, env).value {
            s == "true"
        } else {
            panic!("Invalid argument for or function");
        }
    }) {
        fn_true(&[])
    } else {
        fn_false(&[])
    }
}

pub fn fn_repeat(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Number(n) = evaluate_node(&args[0], env).value {
        (0..n).fold(fn_false(&[]), |_, _| evaluate_node(&args[1], env))
    } else {
        panic!("Invalid argument for repeat function");
    }
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

    fn_true(&[])
}

pub fn fn_debug_write(args: &[Node], env: &mut Environment) -> Node {
    println!("Debug Write:");
    fn_print_env(&[], env);
    println!("Args:");
    for arg in args {
        println!("  Arg: '{}'->'{}'", arg.token.value, arg.string());
    }
    println!(
        "{}",
        evaluate_args!(args, env)
            .iter()
            .map(super::Node::string)
            .collect::<Vec<_>>()
            .join(" ")
            .as_str()
    );

    fn_true(&[])
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

    fn_true(&[])
}

pub fn fn_write_file(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Text(filename) = evaluate_node(&args[0], env).value {
        if let Value::Text(content) = evaluate_node(&args[1], env).value {
            std::fs::write(filename, content).expect("Unable to write file");
            return fn_true(&[]);
        }
    }

    panic!("Invalid arguments for write_file function");
}

pub fn fn_read_file(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Text(filename) = evaluate_node(&args[0], env).value {
        let content = std::fs::read_to_string(filename).expect("Unable to read file");

        Node {
            token: args[0].token.clone(),
            value: Value::Text(content),
            children: Vec::new(),
        }
    } else {
        panic!("Invalid argument for read_file function");
    }
}

pub fn fn_join(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Text(separator) = evaluate_node(&args[0], env).value {
        if let Value::LParen() = &args[1].value {
            let mut elements = Vec::new();
            for child in &args[1].children {
                if let Value::Text(text) = evaluate_node(child, env).value {
                    elements.push(text);
                } else {
                    panic!("Invalid argument for join function");
                }
            }
            let joined = elements.join(&separator);
            return Node {
                token: args[0].token.clone(),
                value: Value::Text(joined),
                children: Vec::new(),
            };
        }
    }

    panic!("Invalid arguments for join function");
}

pub fn fn_split(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Text(separator) = evaluate_node(&args[0], env).value {
        if let Value::Text(string) = evaluate_node(&args[1], env).value {
            let parts = string.split(&separator).map(|s| Node {
                token: args[0].token.clone(),
                value: Value::Text(s.to_string()),
                children: Vec::new(),
            });

            return Node {
                token: args[0].token.clone(),
                value: Value::LParen(),
                children: parts.collect(),
            };
        }
    }

    panic!("Invalid arguments for split function");
}

pub fn fn_lines(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Text(string) = evaluate_node(&args[0], env).value {
        let lines = string.lines().map(|s| Node {
            token: args[0].token.clone(),
            value: Value::Text(s.to_string()),
            children: Vec::new(),
        });

        return Node {
            token: args[0].token.clone(),
            value: Value::LParen(),
            children: lines.collect(),
        };
    }

    panic!("Invalid argument for lines function");
}

pub fn fn_strlen(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Text(string) = evaluate_node(&args[0], env).value {
        return Node {
            token: args[0].token.clone(),
            value: Value::Number(string.len() as i64),
            children: Vec::new(),
        };
    }

    panic!("Invalid argument for strlen function");
}

pub fn fn_empty_string(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Text(string) = evaluate_node(&args[0], env).value {
        if string.is_empty() {
            return fn_true(&[]);
        } else {
            return fn_false(&[]);
        }
    }

    panic!("Invalid argument for empty_string function");
}

// TODO: This function should take an optional argument
pub fn fn_print_env(args: &[Node], env: &mut Environment) -> Node {
    println!("Environment:");
    let red = "\x1b[31m";
    let normal = "\x1b[0m";
    print!("{red}");
    env.variables.iter().for_each(|(key, value)| {
        println!("  {}: {}", key, value.string());
    });
    println!("{normal}");

    fn_true(&[])
}

pub fn test_equal(a: &Node, b: &Node) -> bool {
    if a.value != b.value {
        return false;
    }

    if a.value == Value::LParen() {
        if a.children.len() != b.children.len() {
            return false;
        }
        for (a_child, b_child) in a.children.iter().zip(&b.children) {
            if !test_equal(a_child, b_child) {
                return false;
            }
        }
    }

    true
}

pub fn fn_equal(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    let a = evaluate_node(&args[0], env);
    let b = evaluate_node(&args[1], env);

    if test_equal(&a, &b) {
        fn_true(&[])
    } else {
        fn_false(&[])
    }
}

pub fn fn_if(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 3);

    if let Value::Symbol(ref s) = evaluate_node(&args[0], env).value {
        if s == "true" {
            return evaluate_node(&args[1], env);
        } else {
            return evaluate_node(&args[2], env);
        }
    }

    panic!("Invalid argument for if function");
}

pub fn fn_cond(args: &[Node], env: &mut Environment) -> Node {
    args.iter()
        .find_map(|arg| {
            if let Value::Symbol(ref s) = evaluate_node(&arg.children[0], env).value {
                if s == "true" {
                    Some(evaluate_node(&arg.children[1], env))
                } else {
                    None
                }
            } else {
                panic!("Invalid argument for cond function");
            }
        })
        .unwrap_or_else(|| fn_false(&[]))
}

pub fn fn_less_than(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Number(a) = evaluate_node(&args[0], env).value {
        if let Value::Number(b) = evaluate_node(&args[1], env).value {
            if a < b {
                return fn_true(&[]);
            } else {
                return fn_false(&[]);
            }
        }
    }

    panic!("Invalid arguments for less_than function");
}

pub fn fn_greater_than(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Number(a) = evaluate_node(&args[0], env).value {
        if let Value::Number(b) = evaluate_node(&args[1], env).value {
            if a > b {
                return fn_true(&[]);
            } else {
                return fn_false(&[]);
            }
        }
    }

    panic!("Invalid arguments for greater_than function");
}

pub fn fn_def(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::Symbol(name) = evaluate_node(&args[0], env).value {
        let value = evaluate_node(&args[1], env);
        env.set(name.clone(), value);
        fn_true(&[])
    } else {
        panic!("Invalid argument for def function");
    }
}

fn look_for_bang(children: &[Node]) -> bool {
    children.iter().any(|child| {
        if let Value::Symbol(ref s) = child.value {
            s.ends_with('!')
        } else {
            look_for_bang(&child.children)
        }
    })
}

//fn list_dependencies(name: &str, children: &[Node], env: &mut Environment) {
//    for child in children {
//        if child.token.kind == TokenKind::Symbol {
//            // TODO: Find a way to differentiate between a function and variable
//            println!("{} -> {}", name, child.token.value);
//        } else {
//            list_dependencies(name, &child.children, env);
//        }
//    }
//}

pub fn fn_func(args: &[Node], env: &mut Environment) -> Node {
    ////println!("Creating new function: {}", name);
    ////println!("Dependencies:");
    ////list_dependencies(name, &children, env);

    if let Value::Symbol(name) = evaluate_node(&args[0], env).value {
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
            token: args[0].token.clone(),
            value: Value::Lambda(),
            children,
        };

        env.set(name.clone(), lambda);

        return fn_true(&[]);
    }

    panic!("Invalid argument for func function");
}

pub fn fn_set(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 3);

    if let Value::Symbol(name) = evaluate_node(&args[0], env).value {
        let value = evaluate_node(&args[1], env);
        let body = &args[2];

        let mut new_env = env.clone();
        new_env.set(name.clone(), value);
        return evaluate_node(body, &mut new_env);
    }

    panic!("Invalid argument for set function");
}

// TODO: Should we be trimming?
pub fn fn_read_line(args: &[Node]) -> Node {
    expect_n_args!(args, 0);

    let mut input = String::new();
    std::io::stdin()
        .read_line(&mut input)
        .expect("Failed to read line");

    Node {
        token: args[0].token.clone(),
        value: Value::Text(input.trim().to_string()),
        children: Vec::new(),
    }
}

pub fn fn_list(args: &[Node], env: &mut Environment) -> Node {
    let list = evaluate_args!(args, env);

    Node {
        token: args[0].token.clone(),
        value: Value::LParen(),
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
            token: args[0].token.clone(),
            value: Value::LParen(),
            children: vec![function.clone(), item.clone()],
        })
        .collect::<Vec<_>>()
        .iter()
        .map(|child| evaluate_node(child, env))
        .collect::<Vec<_>>();

    Node {
        token: args[0].token.clone(),
        value: Value::LParen(),
        children,
    }
}

pub fn fn_filter(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    let function = &args[0];
    let list = &args[1];

    let mut children = Vec::new();
    if env.get(&function.token.value.clone()).is_some() {
        // Not intrinsic
        for item in evaluate_node(list, env).children {
            let lambda = handle_symbol(&function, &item.children, env);

            let lambda_params = &lambda.children[0];
            let lambda_body = &lambda.children[1..];

            let mut new_env = env.clone();
            for param in lambda_params.children.iter() {
                new_env.set(param.token.value.clone(), item.clone());
            }

            for child in lambda_body {
                let ret = evaluate_node(child, &mut new_env);
                if ret.token.value == "true" {
                    children.push(item.clone());
                }
            }
        }
    } else {
        // Intrinsic
        for item in evaluate_node(list, env).children {
            let ret = evaluate_node(
                &Node {
                    token: function.token.clone(),
                    value: Value::LParen(),
                    children: vec![function.clone(), item.clone()],
                },
                env,
            );
            if ret.token.value == "true" {
                children.push(item.clone());
            }
        }
    }

    Node {
        token: args[0].token.clone(),
        value: Value::LParen(),
        children,
    }
}

pub fn fn_true(args: &[Node]) -> Node {
    Node {
        token: Token {
            value: "true".to_string(),
            kind: TokenKind::Symbol,
            range: Range { start: 0, end: 0 },
        },
        value: Value::Symbol("true".to_string()),
        children: Vec::new(),
    }
}

pub fn fn_false(args: &[Node]) -> Node {
    Node {
        token: Token {
            value: "false".to_string(),
            kind: TokenKind::Symbol,
            range: Range { start: 0, end: 0 },
        },
        value: Value::Symbol("false".to_string()),
        children: Vec::new(),
    }
}

pub fn fn_is_even(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Number(n) = evaluate_node(&args[0], env).value {
        if n % 2 == 0 {
            return fn_true(&[]);
        } else {
            return fn_false(&[]);
        }
    }

    panic!("Invalid argument for is_even function");
}

pub fn fn_is_odd(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Number(n) = evaluate_node(&args[0], env).value {
        if n % 2 != 0 {
            return fn_true(&[]);
        } else {
            return fn_false(&[]);
        }
    }

    panic!("Invalid argument for is_odd function");
}

pub fn fn_get_environment_variable(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Text(var_name) = evaluate_node(&args[0], env).value {
        let value = std::env::var(var_name).unwrap_or_else(|_| String::from(""));
        return Node {
            token: args[0].token.clone(),
            value: Value::Text(value),
            children: Vec::new(),
        };
    }

    panic!("Invalid argument for get_environment_variable function");
}

pub fn fn_head(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let n = evaluate_node(&args[0], env);
    if let Value::LParen() = n.value {
        if args[0].children.is_empty() {
            panic!("Empty list");
        } else {
            return n.children[0].clone();
        }
    }

    panic!("Invalid argument for head function");
}

pub fn fn_last(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let n = evaluate_node(&args[0], env);
    if let Value::LParen() = n.value {
        if args[0].children.is_empty() {
            panic!("Empty list");
        } else {
            return n.children.last().unwrap().clone();
        }
    }

    panic!("Invalid argument for last function");
}

pub fn fn_tail(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let n = evaluate_node(&args[0], env);
    if let Value::LParen() = n.value {
        if args[0].children.is_empty() {
            panic!("Empty list");
        } else {
            return Node {
                token: args[0].token.clone(),
                value: Value::LParen(),
                children: n.children[1..].to_vec(),
            };
        }
    }

    panic!("Invalid argument for tail function");
}

pub fn fn_length(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let n = evaluate_node(&args[0], env);
    if let Value::LParen() = n.value {
        return Node {
            token: args[0].token.clone(),
            value: Value::Number(n.children.len() as i64),
            children: Vec::new(),
        };
    }

    panic!("Invalid argument for length function");
}

pub fn fn_reverse(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    let n = evaluate_node(&args[0], env);
    if let Value::LParen() = n.value {
        return Node {
            token: args[0].token.clone(),
            value: Value::LParen(),
            children: n.children.iter().rev().cloned().collect(),
        };
    }

    panic!("Invalid argument for reverse function");
}

pub fn fn_load(args: &[Node], env: &mut Environment) -> Node {
    args.iter().for_each(|arg| {
        if let Value::Text(filename) = evaluate_node(arg, env).value {
            process_file(&filename, env, true, false, false);
        } else {
            panic!("Invalid argument for load function");
        }
    });

    fn_true(&[])
}

pub fn fn_url_encode(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Text(input) = evaluate_node(&args[0], env).value {
        let encoded = url::form_urlencoded::byte_serialize(input.as_bytes()).collect::<String>();

        return Node {
            token: args[0].token.clone(),
            value: Value::Text(encoded),
            children: Vec::new(),
        };
    }

    panic!("Invalid argument for url_encode function");
}

pub fn fn_url_decode(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Text(input) = evaluate_node(&args[0], env).value {
        let decoded = url::form_urlencoded::parse(input.as_bytes())
            .map(|(key, value)| format!("{key}={value}"))
            .collect::<Vec<_>>()
            .join("&");

        return Node {
            token: args[0].token.clone(),
            value: Value::Text(decoded),
            children: Vec::new(),
        };
    }

    panic!("Invalid argument for url_decode function");
}

pub fn fn_sleep(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Number(duration) = evaluate_node(&args[0], env).value {
        std::thread::sleep(std::time::Duration::from_secs(
            duration.try_into().expect("Invalid duration"),
        ));
        return fn_true(&[]);
    }

    panic!("Invalid argument for sleep function");
}

pub fn fn_sleep_ms(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Number(duration) = evaluate_node(&args[0], env).value {
        std::thread::sleep(std::time::Duration::from_millis(
            duration.try_into().expect("Invalid duration"),
        ));
        return fn_true(&[]);
    }

    panic!("Invalid argument for sleep_ms function");
}

pub fn fn_time_ms(args: &[Node], env: &mut Environment) -> Node {
    let start = std::time::Instant::now();
    args.iter().for_each(|arg| {
        evaluate_node(arg, env);
    });

    Node {
        token: args[0].token.clone(),
        value: Value::Number(start.elapsed().as_millis() as i64),
        children: Vec::new(),
    }
}

pub fn fn_lambda(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    let params = args[0].clone();
    let body = args[1].clone();

    Node {
        token: args[0].token.clone(),
        value: Value::Lambda(),
        children: vec![params, body],
    }
}

pub fn fn_pipeline(args: &[Node], env: &mut Environment) -> Node {
    let mut root = args[args.len() - 1].clone();

    args.iter()
        .rev()
        .skip(1)
        .fold(&mut root, |current, next_node| {
            current.children.push(next_node.clone());
            current.children.last_mut().unwrap()
        });

    evaluate_node(&root, env)
}

pub fn fn_reverse_pipeline(args: &[Node], env: &mut Environment) -> Node {
    let mut root = args[0].clone();

    args.iter().skip(1).fold(&mut root, |current, next_node| {
        current.children.push(next_node.clone());
        current.children.last_mut().unwrap()
    });

    evaluate_node(&root, env)
}

pub fn fn_block(args: &[Node], env: &mut Environment) -> Node {
    for arg in args {
        evaluate_node(arg, env);
    }

    fn_true(&[])
}

pub fn fn_exit(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 1);

    if let Value::Number(exit_code) = evaluate_node(&args[0], env).value {
        std::process::exit(exit_code.try_into().expect("Invalid exit code"));
    }

    panic!("Invalid argument for exit function");
}

pub fn fn_system(args: &[Node], env: &mut Environment) -> Node {
    if args.is_empty() {
        panic!("No command provided");
    }

    if let Value::Text(command) = evaluate_node(&args[0], env).value {
        let arguments = if args.len() > 1 {
            args[1].clone()
        } else {
            Node {
                token: args[0].token.clone(),
                value: Value::LParen(),
                children: Vec::new(),
            }
        };

        let stdin_string = if args.len() > 2 {
            if let Value::Text(string) = evaluate_node(&args[2], env).value {
                string
            } else {
                panic!("Invalid argument for system function");
            }
        } else {
            String::new()
        };

        let mut child = std::process::Command::new(command)
            .args(arguments.children.iter().map(|arg| {
                if let Value::Text(ref s) = arg.value {
                    s.clone()
                } else {
                    panic!("Invalid argument for system function");
                }
            }))
            .stdin(std::process::Stdio::piped())
            .stdout(std::process::Stdio::piped())
            .stderr(std::process::Stdio::piped())
            .spawn()
            .expect("Failed to start process");

        {
            let mut stdin = child.stdin.take().expect("Failed to open stdin");
            stdin
                .write_all(stdin_string.as_bytes())
                .expect("Failed to write to stdin");
        }

        let output = child
            .wait_with_output()
            .expect("Failed to read stdout/stderr");

        return Node {
            token: args[0].token.clone(),
            value: Value::LParen(),
            children: vec![
                Node {
                    token: args[0].token.clone(),
                    value: Value::Number(output.status.code().unwrap_or(-1) as i64),
                    children: Vec::new(),
                },
                Node {
                    token: args[0].token.clone(),
                    value: Value::Text(String::from_utf8_lossy(&output.stdout).to_string()),
                    children: Vec::new(),
                },
                Node {
                    token: args[0].token.clone(),
                    value: Value::Text(String::from_utf8_lossy(&output.stderr).to_string()),
                    children: Vec::new(),
                },
            ],
        };
    }

    panic!("Invalid argument for system function");
}

pub fn fn_contains(args: &[Node], env: &mut Environment) -> Node {
    expect_n_args!(args, 2);

    if let Value::LParen() = evaluate_node(&args[1], env).value {
        for item in evaluate_node(&args[1], env).children {
            if test_equal(&item, &args[0]) {
                return fn_true(&[]);
            }
        }
    }

    panic!("Invalid arguments for contains function");
}
