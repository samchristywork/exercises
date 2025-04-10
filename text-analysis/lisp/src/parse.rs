use crate::Node;
use crate::Range;
use crate::Token;
use crate::TokenKind;
use crate::Value;

pub fn parse_tokens(tokens: &[Token]) -> Node {
    let mut stack = Vec::new();
    let mut current_node = Node {
        token: tokens[0].clone(),
        value: Value::Module(),
        children: Vec::new(),
    };

    for token in tokens {
        match token.kind {
            TokenKind::LParen => {
                stack.push(current_node);
                current_node = Node {
                    token: token.clone(),
                    value: Value::LParen(),
                    children: Vec::new(),
                };
            }
            TokenKind::RParen => {
                if let Some(mut parent) = stack.pop() {
                    parent.children.push(current_node);
                    current_node = parent;
                } else {
                    panic!("Unmatched right parenthesis");
                }
            }
            TokenKind::Text => current_node.children.push(Node {
                token: token.clone(),
                value: Value::Text(token.value.clone()),
                children: Vec::new(),
            }),
            TokenKind::Symbol => current_node.children.push(Node {
                token: token.clone(),
                value: Value::Symbol(token.value.clone()),
                children: Vec::new(),
            }),
            TokenKind::Number => current_node.children.push(Node {
                token: token.clone(),
                value: Value::Number(token.value.parse().expect("Not a valid number")),
                children: Vec::new(),
            }),
            TokenKind::Atom => current_node.children.push(Node {
                token: token.clone(),
                value: Value::Atom(token.value.clone()),
                children: Vec::new(),
            }),
            TokenKind::Module => current_node.children.push(Node {
                token: token.clone(),
                value: Value::LParen(),
                children: Vec::new(),
            }),
            TokenKind::Lambda => current_node.children.push(Node {
                token: token.clone(),
                value: Value::Lambda(),
                children: Vec::new(),
            }),
        }
    }

    assert!(stack.is_empty(), "Unmatched left parenthesis");

    current_node
}
