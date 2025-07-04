#include "kappok.h"

Lexer *lexer_create(char *source) {
    Lexer *lexer = malloc(sizeof(Lexer));
    if (lexer == NULL) {
        perror("Failed to allocate lexer");
        exit(EXIT_FAILURE);
    }
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    return lexer;
}

void lexer_destroy(Lexer *lexer) {
    free(lexer);
}

void skip_whitespace(Lexer *lexer) {
    while (lexer->source[lexer->pos] && isspace(lexer->source[lexer->pos])) {
        if (lexer->source[lexer->pos] == '\n') {
            lexer->line++;
        }
        lexer->pos++;
    }
}

Token *read_string(Lexer *lexer) {
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        perror("Failed to allocate token");
        exit(EXIT_FAILURE);
    }
    token->type = TOKEN_STRING;
    token->line = lexer->line;
    
    lexer->pos++; // 開始の '"' をスキップ
    
    int start = lexer->pos;
    int len = 0;
    
    while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != '"') {
        lexer->pos++;
        len++;
    }
    
    if (lexer->source[lexer->pos] == '"') {
        lexer->pos++; // 終了の '"' をスキップ
    } else {
        // エラー: 閉じられていない文字列リテラル
        fprintf(stderr, "エラー (行 %d): 閉じられていない文字列リテラルです。\n", token->line);
        token->type = TOKEN_UNKNOWN; // エラーを示す
    }
    
    token->value = malloc(len + 1);
    if (token->value == NULL) {
        perror("Failed to allocate token value");
        exit(EXIT_FAILURE);
    }
    strncpy(token->value, &lexer->source[start], len);
    token->value[len] = '\0';
    
    return token;
}

Token *read_identifier(Lexer *lexer) {
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        perror("Failed to allocate token");
        exit(EXIT_FAILURE);
    }
    token->type = TOKEN_IDENTIFIER;
    token->line = lexer->line;
    
    int start = lexer->pos;
    int len = 0;
    
    while (lexer->source[lexer->pos] && 
           (isalnum(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_')) {
        lexer->pos++;
        len++;
    }
    
    token->value = malloc(len + 1);
    if (token->value == NULL) {
        perror("Failed to allocate token value");
        exit(EXIT_FAILURE);
    }
    strncpy(token->value, &lexer->source[start], len);
    token->value[len] = '\0';

    // キーワードチェック
    if (strcmp(token->value, "def") == 0) {
        token->type = TOKEN_DEF;
    } else if (strcmp(token->value, "return") == 0) {
        token->type = TOKEN_RETURN;
    } else if (strcmp(token->value, "int") == 0) {
        token->type = TOKEN_INT;
    } else if (strcmp(token->value, "str") == 0) {
        token->type = TOKEN_STR;
    } else if (strcmp(token->value, "double") == 0) {
        token->type = TOKEN_DOUBLE;
    } else if (strcmp(token->value, "bool") == 0) {
        token->type = TOKEN_BOOL;
    } else if (strcmp(token->value, "True") == 0) {
        token->type = TOKEN_TRUE;
    } else if (strcmp(token->value, "False") == 0) {
        token->type = TOKEN_FALSE;
    }
    
    return token;
}

// 数値を読み取る関数
Token *read_number(Lexer *lexer) {
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        perror("Failed to allocate token");
        exit(EXIT_FAILURE);
    }
    token->type = TOKEN_NUMBER; // まずは整数として初期化
    token->line = lexer->line;

    int start = lexer->pos;
    int len = 0;
    bool is_float = false;

    while (lexer->source[lexer->pos] && isdigit(lexer->source[lexer->pos])) {
        lexer->pos++;
        len++;
    }

    // 小数点があるかチェック
    if (lexer->source[lexer->pos] == '.') {
        is_float = true;
        lexer->pos++; // '.' をスキップ
        len++;

        // 小数点以下の数字を読み込む
        while (lexer->source[lexer->pos] && isdigit(lexer->source[lexer->pos])) {
            lexer->pos++;
            len++;
        }
    }

    // 浮動小数点数であればタイプを更新
    if (is_float) {
        token->type = TOKEN_FLOAT_LITERAL;
    }

    token->value = malloc(len + 1);
    if (token->value == NULL) {
        perror("Failed to allocate token value");
        exit(EXIT_FAILURE);
    }
    strncpy(token->value, &lexer->source[start], len);
    token->value[len] = '\0';

    return token;
}


Token *lexer_next_token(Lexer *lexer) {
    skip_whitespace(lexer);
    
    if (!lexer->source[lexer->pos]) {
        Token *token = malloc(sizeof(Token));
        if (token == NULL) {
            perror("Failed to allocate token");
            exit(EXIT_FAILURE);
        }
        token->type = TOKEN_EOF;
        token->value = NULL;
        token->line = lexer->line;
        return token;
    }
    
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        perror("Failed to allocate token");
        exit(EXIT_FAILURE);
    }
    token->line = lexer->line;
    
    switch (lexer->source[lexer->pos]) {
        case '(':
            token->type = TOKEN_LPAREN;
            token->value = strdup("(");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case ')':
            token->type = TOKEN_RPAREN;
            token->value = strdup(")");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case ',':
            token->type = TOKEN_COMMA;
            token->value = strdup(",");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case '{':
            token->type = TOKEN_LBRACE;
            token->value = strdup("{");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case '}':
            token->type = TOKEN_RBRACE;
            token->value = strdup("}");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case '=':
            token->type = TOKEN_ASSIGN;
            token->value = strdup("=");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case '+': 
            token->type = TOKEN_PLUS;
            token->value = strdup("+");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case '-': 
            token->type = TOKEN_MINUS;
            token->value = strdup("-");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case '*': 
            token->type = TOKEN_ASTERISK;
            token->value = strdup("*");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case '/': 
            token->type = TOKEN_SLASH;
            token->value = strdup("/");
            if (token->value == NULL) { perror("strdup failed"); exit(EXIT_FAILURE); }
            lexer->pos++;
            break;
        case '"':
            token_destroy(token);
            return read_string(lexer);
        default:
            if (isalpha(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_') {
                token_destroy(token);
                return read_identifier(lexer);
            } else if (isdigit(lexer->source[lexer->pos])) { // 数字の開始
                token_destroy(token);
                return read_number(lexer);
            }
            else {
                token->type = TOKEN_UNKNOWN;
                token->value = malloc(2);
                if (token->value == NULL) {
                    perror("Failed to allocate token value");
                    exit(EXIT_FAILURE);
                }
                token->value[0] = lexer->source[lexer->pos];
                token->value[1] = '\0';
                fprintf(stderr, "エラー (行 %d): 不明な文字 '%c' です。\n", token->line, lexer->source[lexer->pos]);
                lexer->pos++;
            }
    }
    
    return token;
}

void token_destroy(Token *token) {
    if (token) {
        if (token->value) {
            free(token->value);
        }
        free(token);
    }
}