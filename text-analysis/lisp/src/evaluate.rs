use crate::Environment;
use crate::Node;
use crate::TokenKind;
use crate::intrinsic;

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

fn handle_symbol(function: &Node, args: &[Node], env: &mut Environment) -> Node {
    match function.token.value.as_str() {
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
        "true" => intrinsic::fn_true(),
        "false" => intrinsic::fn_false(),

        // I/O
        "write!" => intrinsic::fn_write(args, env),
        "write-stderr!" => intrinsic::fn_write_stderr(args, env),
        "write-file!" => intrinsic::fn_write_file(args, env),
        "read-line!" => intrinsic::fn_read_line(),
        "read-file!" => intrinsic::fn_read_file(args, env),

        // Strings
        "join" => intrinsic::fn_join(args, env),
        "split" => intrinsic::fn_split(args, env),
        "empty-string?" => intrinsic::fn_empty_string(args, env),

        // Environment
        "print-env!" => intrinsic::fn_print_env(args, env),

        // Assignment
        "def!" => intrinsic::fn_def(args, env),
        "func!" => intrinsic::fn_func(args, env),

        // Higher-order functions
        "map" => intrinsic::fn_map(args, env),
        "filter" => intrinsic::fn_filter(args, env),

        // Lists
        "list" => intrinsic::fn_list(args, env),
        "head" => intrinsic::fn_head(args, env),
        "tail" => intrinsic::fn_tail(args, env),
        "length" => intrinsic::fn_length(args, env),

        // Miscellaneous
        "get-environment-variable!" => intrinsic::fn_get_environment_variable(args, env),
        "even?" => intrinsic::fn_is_even(args, env),
        "odd?" => intrinsic::fn_is_odd(args, env),
        "url-encode" => intrinsic::fn_url_encode(args, env),
        "url-decode" => intrinsic::fn_url_decode(args, env),
        "load!" => intrinsic::fn_load(args, env),
        "sleep" => intrinsic::fn_sleep(args, env),
        "sleep-ms" => intrinsic::fn_sleep_ms(args, env),
        _ => env.get(&function.token.value).map_or_else(
            || panic!("Unknown function: {}", function.token.value),
            std::clone::Clone::clone,
        ),
    }
}

fn apply_function(function: &Node, args: &[Node], env: &mut Environment) -> Node {
    match function.token.kind {
        TokenKind::Symbol => handle_symbol(function, args, env),
        TokenKind::Lambda => {
            let params = &function.children[0];
            let body = &function.children[1..];

            assert_eq!(params.children.len(), args.len(), "Argument count mismatch");

            let mut new_env = env.clone();
            for (param, arg) in params.children.iter().zip(args) {
                new_env.set(param.token.value.clone(), evaluate_node(arg, env));
            }

            let mut return_value = intrinsic::fn_true();
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
