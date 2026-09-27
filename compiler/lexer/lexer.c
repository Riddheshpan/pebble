#include "lexer.h"
#include <string.h>

static TokenKind identifier_type(const Lexer *lexer);
static Token number(Lexer *lexer);
static Token string_literal(Lexer *lexer);
static Token char_literal(Lexer *lexer);
static int match(Lexer *lexer, char expected);
static Token operator_or_punctuation(Lexer *lexer, char c);
static Token make_error_token(const Lexer *lexer, const char *message);

static int is_at_end(const Lexer *lexer)
{
    return lexer->source[lexer->current] == '\0';
}

static char advance(Lexer *lexer)
{
    char c = lexer->source[lexer->current];

    if (c != '\0') {
        lexer->current++;

        if (c == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
    }

    return c;
}

static char peek(const Lexer *lexer)
{
    return lexer->source[lexer->current];
}

static Token make_token(const Lexer *lexer, TokenKind kind)
{
    Token token;

    token.kind = kind;
    token.start = lexer->source + lexer->start;
    token.length = lexer->current - lexer->start;
    token.line = lexer->line;
    token.column = lexer->column - token.length;
    token.error_message = NULL;

    return token;
}

static Token make_error_token(const Lexer *lexer, const char *message)
{
    Token token = make_token(lexer, TOKEN_ERROR);
    token.error_message = message;
    return token;
}

static void skip_whitespace(Lexer *lexer)
{
    for (;;) {
        char c = peek(lexer);

        switch (c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                advance(lexer);
                break;

            case '/':
                if (lexer->source[lexer->current + 1] == '/') {
                    /* Line comment */
                    while (!is_at_end(lexer) && peek(lexer) != '\n') {
                        advance(lexer);
                    }
                } else if (lexer->source[lexer->current + 1] == '*') {
                    /* Block comment */
                    advance(lexer); /* / */
                    advance(lexer); /* * */

                    while (!is_at_end(lexer)) {
                        if (peek(lexer) == '*' &&
                            lexer->source[lexer->current + 1] == '/') {
                            advance(lexer); /* * */
                            advance(lexer); /* / */
                            break;
                        }

                        advance(lexer);
                    }
                } else {
                    return;
                }
                break;

            default:
                return;
        }
    }
}

void lexer_init(Lexer *lexer, const char *source)
{
    lexer->source = source;

    lexer->start = 0;
    lexer->current = 0;

    lexer->line = 1;
    lexer->column = 1;
}

Token lexer_next_token(Lexer *lexer)
{
    skip_whitespace(lexer);

    lexer->start = lexer->current;

    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF);
    }

    char c = advance(lexer);
    if (c == '"') {
        return string_literal(lexer);
    }

    if (c == '\'') {
        return char_literal(lexer);
    }    

    if (c >= '0' && c <= '9') {
        return number(lexer);
    }

    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        c == '_') {

        while (!is_at_end(lexer)) {
            char next = peek(lexer);

            if (!((next >= 'a' && next <= 'z') ||
                  (next >= 'A' && next <= 'Z') ||
                  (next >= '0' && next <= '9') ||
                  next == '_')) {
                break;
            }

            advance(lexer);
        }

        return make_token(lexer, identifier_type(lexer));
    }

    return operator_or_punctuation(lexer, c);
}

static TokenKind identifier_type(const Lexer *lexer)
{
    size_t length = lexer->current - lexer->start;
    const char *text = lexer->source + lexer->start;

#define KEYWORD(word, token) \
    if (length == sizeof(word) - 1 && \
        strncmp(text, word, sizeof(word) - 1) == 0) \
        return token;

    KEYWORD("import", TOKEN_IMPORT)
    KEYWORD("function", TOKEN_FUNCTION)
    KEYWORD("struct", TOKEN_STRUCT)
    KEYWORD("enum", TOKEN_ENUM)
    KEYWORD("const", TOKEN_CONST)

    KEYWORD("number", TOKEN_NUMBER)
    KEYWORD("decimal", TOKEN_DECIMAL)
    KEYWORD("bool", TOKEN_BOOL)
    KEYWORD("char", TOKEN_CHAR)
    KEYWORD("string", TOKEN_STRING)
    KEYWORD("void", TOKEN_VOID)

    KEYWORD("if", TOKEN_IF)
    KEYWORD("else", TOKEN_ELSE)
    KEYWORD("while", TOKEN_WHILE)
    KEYWORD("repeat", TOKEN_REPEAT)
    KEYWORD("for", TOKEN_FOR)

    KEYWORD("break", TOKEN_BREAK)
    KEYWORD("continue", TOKEN_CONTINUE)
    KEYWORD("return", TOKEN_RETURN)

    KEYWORD("true", TOKEN_TRUE)
    KEYWORD("false", TOKEN_FALSE)
    KEYWORD("print", TOKEN_PRINT)

#undef KEYWORD

    return TOKEN_IDENTIFIER;
}

static Token number(Lexer *lexer)
{
    while (peek(lexer) >= '0' && peek(lexer) <= '9') {
        advance(lexer);
    }

    if (peek(lexer) == '.' &&
        lexer->source[lexer->current + 1] >= '0' &&
        lexer->source[lexer->current + 1] <= '9') {

        advance(lexer);

        while (peek(lexer) >= '0' && peek(lexer) <= '9') {
            advance(lexer);
        }

        return make_token(lexer, TOKEN_DECIMAL_LITERAL);
    }

    return make_token(lexer, TOKEN_NUMBER_LITERAL);
}

static Token string_literal(Lexer *lexer)
{
    while (!is_at_end(lexer) && peek(lexer) != '"') {
        advance(lexer);
    }

    if (is_at_end(lexer)) {
        return make_error_token(lexer, "unterminated string");
    }

    advance(lexer);

    return make_token(lexer, TOKEN_STRING_LITERAL);
}

static Token char_literal(Lexer *lexer)
{
    if (is_at_end(lexer) || peek(lexer) == '\n') {
        return make_error_token(lexer, "unterminated character literal");
    }

    if (peek(lexer) == '\\') {
        advance(lexer);

        if (is_at_end(lexer)) {
            return make_error_token(lexer, "unterminated character literal");
        }

        advance(lexer);
    } else {
        advance(lexer);
    }

    if (peek(lexer) != '\'') {
        return make_token(lexer, TOKEN_ERROR);
    }

    advance(lexer);

    return make_token(lexer, TOKEN_CHAR_LITERAL);
}

static int match(Lexer *lexer, char expected)
{
    if (is_at_end(lexer)) {
        return 0;
    }

    if (peek(lexer) != expected) {
        return 0;
    }

    advance(lexer);
    return 1;
}

static Token operator_or_punctuation(Lexer *lexer, char c)
{
    switch (c) {
        case '+':
            return make_token(lexer, TOKEN_PLUS);

        case '-':
            return make_token(lexer, TOKEN_MINUS);

        case '*':
            return make_token(lexer, TOKEN_STAR);

        case '/':
            return make_token(lexer, TOKEN_SLASH);

        case '%':
            return make_token(lexer, TOKEN_PERCENT);

        case '=':
            return make_token(
                lexer,
                match(lexer, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
            );

        case '!':
            return make_token(
                lexer,
                match(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG
            );

        case '<':
            return make_token(
                lexer,
                match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS
            );

        case '>':
            return make_token(
                lexer,
                match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
            );

        case '&':
            if (match(lexer, '&')) {
                return make_token(lexer, TOKEN_AND_AND);
            }
            return make_token(lexer, TOKEN_ERROR);

        case '|':
            if (match(lexer, '|')) {
                return make_token(lexer, TOKEN_OR_OR);
            }
            return make_token(lexer, TOKEN_ERROR);

        case '(':
            return make_token(lexer, TOKEN_LPAREN);

        case ')':
            return make_token(lexer, TOKEN_RPAREN);

        case '{':
            return make_token(lexer, TOKEN_LBRACE);

        case '}':
            return make_token(lexer, TOKEN_RBRACE);

        case '[':
            return make_token(lexer, TOKEN_LBRACKET);

        case ']':
            return make_token(lexer, TOKEN_RBRACKET);

        case ',':
            return make_token(lexer, TOKEN_COMMA);

        case ';':
            return make_token(lexer, TOKEN_SEMICOLON);

        case '.':
            return make_token(lexer, TOKEN_DOT);

        default:
            return make_token(lexer, TOKEN_ERROR);
    }
}