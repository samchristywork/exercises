use crate::Environment;
use crate::Node;
use crate::TokenKind;
use crate::Value;
use crate::intrinsic;

const VERBOSE: bool = false;

pub fn evaluate_node(node: &Node, env: &mut Environment) -> Node {
    if VERBOSE {
        println!("Evaluating node: {}", node);
    }

    match &node.value {
        Value::LParen() => {
            let function = evaluate_node(&node.children[0], env);
            apply_function(&function, &node.children[1..], env)
        }
        Value::Symbol(s) => env
            .get(&s)
            .map_or_else(|| node.clone(), std::clone::Clone::clone),
        Value::Module() => {
            let mut result = node.clone();
            for child in &node.children {
                result = evaluate_node(child, env);
            }
            result
        }
        _ => node.clone(),
    }
}

pub fn handle_symbol(function: &Node, args: &[Node], env: &mut Environment) -> Node {
    if VERBOSE {
        println!("Handling symbol: {}", function);
    }

    let name = match &function.value {
        Value::Symbol(s) => s,
        _ => panic!("Expected a symbol, found: {:?}", function.value),
    };

    match name.as_str() {
        // Operators
        "+" => intrinsic::fn_add(args, env),
        "-" => intrinsic::fn_sub(args, env),
        "*" => intrinsic::fn_mul(args, env),
        "=" => intrinsic::fn_equal(args, env),
        "<" => intrinsic::fn_less_than(args, env),
        ">" => intrinsic::fn_greater_than(args, env),
        "^" => intrinsic::fn_pow(args, env),
        "%" => intrinsic::fn_mod(args, env),
        "++" => intrinsic::fn_inc(args, env),
        "--" => intrinsic::fn_dec(args, env),
        "max" => intrinsic::fn_max(args, env),
        "min" => intrinsic::fn_min(args, env),

        // Types
        "text?" => intrinsic::fn_is_text(args, env),
        "number?" => intrinsic::fn_is_number(args, env),
        "symbol?" => intrinsic::fn_is_symbol(args, env),
        "list?" => intrinsic::fn_is_lparen(args, env),
        "lambda?" => intrinsic::fn_is_lambda(args, env),
        "atom?" => intrinsic::fn_is_atom(args, env),

        // Logical
        "&&" => intrinsic::fn_and(args, env),
        "||" => intrinsic::fn_or(args, env),

        // Control flow
        "if" => intrinsic::fn_if(args, env),
        "cond" => intrinsic::fn_cond(args, env),
        "repeat" => intrinsic::fn_repeat(args, env),
        "loop" => intrinsic::fn_loop(args, env),
        "true" => intrinsic::fn_true(args),
        "false" => intrinsic::fn_false(args),
        "|" => intrinsic::fn_pipeline(args, env),
        "rev|" => intrinsic::fn_reverse_pipeline(args, env),
        "{}" => intrinsic::fn_block(args, env),
        "exit" => intrinsic::fn_exit(args, env), // TODO: Is ! needed?

        // I/O
        "write!" => intrinsic::fn_write(args, env),
        "!" => intrinsic::fn_debug_write(args, env),
        "write-stderr!" => intrinsic::fn_write_stderr(args, env),
        "write-file!" => intrinsic::fn_write_file(args, env),
        "read-line!" => intrinsic::fn_read_line(args),
        "read-file!" => intrinsic::fn_read_file(args, env),

        // Strings
        "join" => intrinsic::fn_join(args, env),
        "split" => intrinsic::fn_split(args, env),
        "lines" => intrinsic::fn_lines(args, env),
        "strlen" => intrinsic::fn_strlen(args, env),
        "empty-string?" => intrinsic::fn_empty_string(args, env),

        // Environment
        "print-env!" => intrinsic::fn_print_env(args, env),

        // Assignment
        "def!" => intrinsic::fn_def(args, env),
        "func!" => intrinsic::fn_func(args, env),
        "set" => intrinsic::fn_set(args, env),
        "lambda" => intrinsic::fn_lambda(args, env), // TODO: Is ! needed?

        // Higher-order functions
        "map" => intrinsic::fn_map(args, env),
        "filter" => intrinsic::fn_filter(args, env),

        // Lists
        "list" | "'" => intrinsic::fn_list(args, env),
        "head" => intrinsic::fn_head(args, env),
        "last" => intrinsic::fn_last(args, env),
        "tail" => intrinsic::fn_tail(args, env),
        "length" => intrinsic::fn_length(args, env),
        "reverse" => intrinsic::fn_reverse(args, env),

        // Miscellaneous
        "get-environment-variable!" => intrinsic::fn_get_environment_variable(args, env),
        "even?" => intrinsic::fn_is_even(args, env),
        "odd?" => intrinsic::fn_is_odd(args, env),
        "url-encode" => intrinsic::fn_url_encode(args, env),
        "url-decode" => intrinsic::fn_url_decode(args, env),
        "load!" => intrinsic::fn_load(args, env),
        "sleep" => intrinsic::fn_sleep(args, env),
        "sleep-ms" => intrinsic::fn_sleep_ms(args, env),
        "time-ms" => intrinsic::fn_time_ms(args, env),
        "system!" => intrinsic::fn_system(args, env),
        "contains" => intrinsic::fn_contains(args, env),
        _ => env.get(&name).map_or_else(
            || panic!("Unknown function: {}", name),
            std::clone::Clone::clone,
        ),
    }
}

fn apply_function(function: &Node, args: &[Node], env: &mut Environment) -> Node {
    if VERBOSE {
        println!("Applying function: {}", function);
        for arg in args {
            println!("Argument: {}", arg);
        }
    }
    match function.value {
        Value::Symbol(_) => handle_symbol(function, args, env),
        Value::Lambda() => {
            let params = &function.children[0];
            let body = &function.children[1..];

            assert_eq!(params.children.len(), args.len(), "Argument count mismatch");

            let mut new_env = env.clone();
            for (param, arg) in params.children.iter().zip(args) {
                match param.value {
                    Value::Symbol(ref s) => new_env.set(s.clone(), evaluate_node(arg, env)),
                    _ => panic!("Expected a symbol, found: {:?}", param.value),
                }
            }

            let mut return_value = intrinsic::fn_true(&[]);
            for child in body {
                return_value = evaluate_node(child, &mut new_env);
            }

            return_value
        }
        Value::LParen() => {
            apply_function(&function.children[0].clone(), &function.children[1..], env)
        }
        _ => panic!("Invalid function application: {:?}", function.value),
    }
}
