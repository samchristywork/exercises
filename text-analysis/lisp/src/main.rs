mod evaluate;
mod parse;
mod tokenize;
mod intrinsic;

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
    Lambda,
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
                TokenKind::Lambda => format!("Lambda: {}", self.value),
            }
        )
    }
}

#[derive(Clone)]
struct Node {
    token: Token,
    children: Vec<Node>,
}

impl Node {
    fn string(&self) -> String {
        match self.token.kind {
            TokenKind::Text => self.token.value.clone(),
            TokenKind::Number => self.token.value.clone(),
            TokenKind::Symbol => self.token.value.clone(),
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

#[derive(Clone)]
struct Environment {
    variables: std::collections::HashMap<String, Node>,
}

impl fmt::Display for Environment {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for (key, value) in &self.variables {
            write!(f, "{}: {}\n", key, value)?;
        }
        Ok(())
    }
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
    let tokens = tokenize(&source);
    let ast = parse_tokens(&tokens);
    evaluate_node(&ast, &mut Environment::new());
}
