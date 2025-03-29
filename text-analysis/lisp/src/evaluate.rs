use crate::Environment;
use crate::Node;
use crate::TokenKind;
use crate::intrinsic;

fn apply_function(function: &Node, args: &[Node], env: &mut Environment) -> Node {
    match function.token.kind {
        TokenKind::Symbol => match function.token.value.as_str() {
            "+" => intrinsic::fn_add(args, env),
            "-" => intrinsic::fn_sub(args, env),
            "*" => intrinsic::fn_mul(args, env),
            "repeat" => intrinsic::fn_repeat(args, env),
            "loop" => intrinsic::fn_loop(args, env),
            "print" => intrinsic::fn_print(args, env),
            "join" => intrinsic::fn_join(args, env),
            "print-env" => intrinsic::fn_print_env(args, env),
            "true" => intrinsic::fn_true(),
            "false" => intrinsic::fn_false(),
            "=" => intrinsic::fn_equal(args, env),
            "if" => intrinsic::fn_if(args, env),
            "cond" => intrinsic::fn_cond(args, env),
            "<" => intrinsic::fn_less_than(args, env),
            ">" => intrinsic::fn_greater_than(args, env),
            "def" => intrinsic::fn_def(args, env),
            "defun" => intrinsic::fn_defun(args, env),
            "read-line" => intrinsic::fn_read_line(),
            "list" => intrinsic::fn_list(args, env),
            "map" => intrinsic::fn_map(args, env),
            _ => env.get(&function.token.value).map_or_else(
                || panic!("Unknown function: {}", function.token.value),
                std::clone::Clone::clone,
            ),
        },
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
