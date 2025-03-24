mod evaluate;
mod parse;
mod tokenize;

use std::fmt;

use crate::evaluate::evaluate_node;
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

#[derive(Clone)]
enum TokenKind {
    Text,
    Number,
    Symbol,
    LParen,
    RParen,
    Module,
}

#[derive(Clone)]
struct Token {
    value: String,
    kind: TokenKind,
    range: Range,
}

impl fmt::Display for Token {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{}: {}",
            self.range,
            match self.kind {
                TokenKind::Module => format!("Module: {}", self.value),
                TokenKind::Text => format!("String: {}", self.value),
                TokenKind::Number => format!("Number: {}", self.value),
                TokenKind::Symbol => format!("Symbol: {}", self.value),
                TokenKind::LParen => format!("Left Parenthesis: {}", self.value),
                TokenKind::RParen => format!("Right Parenthesis: {}", self.value),
            }
        )
    }
}

#[derive(Clone)]
struct Node {
    token: Token,
    children: Vec<Node>,
}

// TODO
fn to_string_indent(node: &Node, indent: usize) -> String {
    let mut result = String::new();
    for _ in 0..indent {
        result.push(' ');
    }
    result.push_str(&node.token.value);
    result.push('\n');
    for child in &node.children {
        result.push_str(&to_string_indent(child, indent + 2));
    }
    result
}

impl fmt::Display for Node {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", to_string_indent(self, 0))
    }
}

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

fn read_file(filename: &str) -> String {
    use std::fs;
    fs::read_to_string(filename).expect("Unable to read file")
}

fn main() {
    let filename = std::env::args().nth(1).expect("No filename provided");
    let source = read_file(&filename);

    println!("Tokenizing");
    let tokens = tokenize(&source);

    println!("Parsing");
    let ast = parse_tokens(&tokens);

    println!("Evaluating");
    evaluate_node(&ast, &mut Environment::new());
}
