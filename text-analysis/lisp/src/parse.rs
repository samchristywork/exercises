use crate::Node;
use crate::Range;
use crate::Token;
use crate::TokenKind;

pub fn parse_tokens(tokens: &[Token]) -> Node {
    let mut stack = Vec::new();
    let mut current_node = Node {
        token: Token {
            value: String::from("Root"),
            kind: TokenKind::Module,
            range: Range { start: 0, end: 0 },
        },
        children: Vec::new(),
    };

    for token in tokens {
        match token.kind {
            TokenKind::LParen => {
                stack.push(current_node);
                current_node = Node {
                    token: token.clone(),
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
            _ => {
                current_node.children.push(Node {
                    token: token.clone(),
                    children: Vec::new(),
                });
            }
        }
    }

    assert!(stack.is_empty(), "Unmatched left parenthesis");

    current_node
}
