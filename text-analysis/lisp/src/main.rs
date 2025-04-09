use std::fs;
use std::io::Write;

mod evaluate;
mod intrinsic;
mod parse;
mod tokenize;

use std::fmt;

use crate::evaluate::evaluate_node;
use crate::evaluate::handle_symbol;
use crate::parse::parse_tokens;
use crate::tokenize::tokenize;

#[derive(Clone)]
struct Range {
    start: usize,
    end: usize,
}

impl fmt::Display for Range {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}..{}", self.start, self.end)
    }
}

#[derive(Clone, PartialEq, Debug)]
enum TokenKind {
    Text,
    Number,
    Symbol,
    LParen,
    RParen,
    Module,
    Lambda,
    Atom,
}

#[derive(Clone)]
struct Token {
    value: String,
    kind: TokenKind,
    range: Range,
}

const MAGENTA: &str = "\x1b[35m";
const YELLOW: &str = "\x1b[33m";
const NORMAL: &str = "\x1b[0m";
const GREY: &str = "\x1b[90m";
const CYAN: &str = "\x1b[36m";

impl fmt::Display for Token {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{MAGENTA}{}: {YELLOW}{}{NORMAL}",
            self.range,
            match self.kind {
                TokenKind::Module => format!("Module: {}", self.value),
                TokenKind::Text => format!("String: \"{}\"", self.value),
                TokenKind::Number => format!("Number: {}", self.value),
                TokenKind::Symbol => format!("Symbol: {}", self.value),
                TokenKind::LParen => format!("Left Parenthesis: {}", self.value),
                TokenKind::RParen => format!("Right Parenthesis: {}", self.value),
                TokenKind::Lambda => format!("Lambda: {}", self.value),
                TokenKind::Atom => format!("Atom: {}", self.value),
            }
        )
    }
}

#[derive(Clone)]
struct Node {
    token: Token,
    children: Vec<Node>,
}

fn print_tree(node: &Node, depth: usize) -> String {
    let mut result = String::new();
    for _ in 0..depth {
        result.push_str("  ");
    }
    if node.token.kind == TokenKind::Text {
        result.push_str(&format!("\"{}\"", node.token.value));
    } else {
        result.push_str(&node.token.value);
    }
    result.push('\n');
    for child in &node.children {
        result.push_str(&print_tree(child, depth + 1));
    }
    if node.token.kind == TokenKind::LParen {
        for _ in 0..depth {
            result.push_str("  ");
        }
        result.push_str(")\n");
    }
    result
}

impl fmt::Display for Node {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", print_tree(self, 0))
    }
}

impl Node {
    fn string(&self) -> String {
        match self.token.kind {
            TokenKind::LParen => {
                let mut result = String::new();
                for child in &self.children {
                    result.push_str(&child.string());
                    result.push(' ');
                }
                result.trim_end().to_string()
            }
            _ => self.token.value.clone(),
        }
    }
}

#[derive(Clone)]
struct Environment {
    variables: std::collections::HashMap<String, Node>,
}

impl Environment {
    fn new() -> Self {
        Self {
            variables: std::collections::HashMap::new(),
        }
    }

    fn get(&self, name: &str) -> Option<&Node> {
        self.variables.get(name)
    }

    fn set(&mut self, name: String, value: Node) {
        self.variables.insert(name, value);
    }
}

fn process_file(filename: &str, env: &mut Environment, script: bool, print_tokens: bool, print_ast: bool) {
    let mut source = fs::read_to_string(filename).expect("Unable to read file");
    if script && source.starts_with("#!") {
        if let Some(new_line_index) = source.find('\n') {
            source = source[new_line_index + 1..].to_string();
        }
    }

    let tokens = tokenize(&source);
    if print_tokens {
        println!("\n{GREY}Tokens:{NORMAL}");
        for token in &tokens {
            println!("{CYAN}{token}{NORMAL}");
        }
    }
    let ast = parse_tokens(&tokens);
    if print_ast {
        println!("\n{GREY}AST:{NORMAL}\n{ast}");
    }
    evaluate_node(&ast, env);
}

fn main() {
    let mut env = Environment::new();

    let flags: Vec<String> = std::env::args()
        .skip(1)
        .filter(|arg| arg.starts_with('-'))
        .collect();
    let positional_args: Vec<String> = std::env::args()
        .skip(1)
        .filter(|arg| !arg.starts_with('-'))
        .collect();

    let mut repl = false;
    let mut script = false;
    let mut print_tokens = false;
    let mut print_ast = false;

    for flag in &flags {
        if flag == "--repl" || flag == "-r" {
            repl = true;
        } else if flag == "--script" || flag == "-s" {
            script = true;
        } else if flag == "--print-tokens" || flag == "-t" {
            print_tokens = true;
        } else if flag == "--print-ast" || flag == "-a" {
            print_ast = true;
        } else if flag == "--version" || flag == "-v" {
            println!("Lich version 2025.4.1");
            return;
        } else if flag == "--help" || flag == "-h" {
            println!("Usage: lich [options] [file]");
            println!("Options:");
            println!("  -r, --repl             Start the REPL");
            println!("  -s, --script           Run the script");
            println!("  -t, --print-tokens     Print tokens");
            println!("  -a, --print-ast        Print AST");
            println!("  -v, --version          Show version");
            println!("  -h, --help             Show this help message");
            return;
        } else {
            println!("Unknown flag: {flag}");
        }
    }

    if repl {
        println!("Starting REPL...");
        loop {
            let mut input = String::new();
            print!("> ");
            std::io::stdout().flush().expect("Failed to flush stdout");
            std::io::stdin()
                .read_line(&mut input)
                .expect("Failed to read line");
            let input = input.trim();
            if input == "exit" || input.is_empty() {
                break;
            }
            let tokens = tokenize(input);
            if print_tokens {
                println!("\n{GREY}Tokens:{NORMAL}");
                for token in &tokens {
                    println!("{token}");
                }
            }
            let ast = parse_tokens(&tokens);
            if print_ast {
                println!("\n{GREY}AST:{NORMAL}");
                println!("{CYAN}{ast}{NORMAL}");
            }

            evaluate_node(&ast, &mut env);
        }
    } else {
        for arg in positional_args {
            process_file(&arg, &mut env, script, print_tokens, print_ast);
        }
    }
}
