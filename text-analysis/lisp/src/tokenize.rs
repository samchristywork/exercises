use crate::Range;
use crate::Token;
use crate::TokenKind;

fn is_symbol_char(c: char) -> bool {
    c.is_alphanumeric()
        || c == '%'
        || c == '*'
        || c == '+'
        || c == '-'
        || c == '/'
        || c == '<'
        || c == '='
        || c == '>'
        || c == '?'
        || c == '^'
        || c == '_'
}

pub fn tokenize(source: &str) -> Vec<Token> {
    let mut tokens = Vec::new();
    let mut chars = source.char_indices().peekable();

    while let Some((start, c)) = chars.next() {
        match c {
            c if c.is_whitespace() => {} // Skip whitespace
            ';' => {
                // Skip comment until newline
                for (_, c) in chars.by_ref() {
                    if c == '\n' {
                        break;
                    }
                }
            }
            '(' => {
                tokens.push(Token {
                    value: "(".to_string(),
                    kind: TokenKind::LParen,
                    range: Range { start, end: start },
                });
            }
            ')' => {
                tokens.push(Token {
                    value: ")".to_string(),
                    kind: TokenKind::RParen,
                    range: Range { start, end: start },
                });
            }
            c if c.is_ascii_digit() => {
                let mut value = String::new();
                value.push(c);
                let mut end = start + 1;

                while let Some(&(next_start, next_c)) = chars.peek() {
                    if next_c.is_ascii_digit() {
                        value.push(next_c);
                        chars.next();
                        end = next_start + 1;
                    } else {
                        break;
                    }
                }
                tokens.push(Token {
                    value,
                    kind: TokenKind::Number,
                    range: Range { start, end },
                });
            }
            c if is_symbol_char(c) => {
                let mut value = String::new();
                value.push(c);
                let mut end = start + 1;

                while let Some(&(next_start, next_c)) = chars.peek() {
                    if is_symbol_char(next_c) {
                        value.push(next_c);
                        chars.next();
                        end = next_start + 1;
                    } else {
                        break;
                    }
                }
                tokens.push(Token {
                    value,
                    kind: TokenKind::Symbol,
                    range: Range { start, end },
                });
            }
            ':' => {
                let mut value = String::new();
                value.push(c);
                let mut end = start + 1;

                while let Some(&(next_start, next_c)) = chars.peek() {
                    if is_symbol_char(next_c) {
                        value.push(next_c);
                        chars.next();
                        end = next_start + 1;
                    } else {
                        break;
                    }
                }
                tokens.push(Token {
                    value,
                    kind: TokenKind::Atom,
                    range: Range { start, end },
                });
            }
            '"' => {
                let mut value = String::new();
                let mut end = start + 1;

                while let Some(&(next_start, next_c)) = chars.peek() {
                    chars.next();
                    end = next_start + 1;
                    if next_c == '"' {
                        break;
                    }

                    value.push(next_c);
                }
                tokens.push(Token {
                    value,
                    kind: TokenKind::Text,
                    range: Range { start, end },
                });
            }
            _ => panic!("Unexpected character: {c}"),
        }
    }

    tokens
}
