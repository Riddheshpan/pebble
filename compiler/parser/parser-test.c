#include <stdio.h>

#include "parser.h"

int main(void)
{
    const char *source =
        "struct Point {"
            "number x;"
            "number y;"
        "}"
        "struct Matrix {"
            "number[10][20] data;"
        "}"
        "function number[10][20] create_matrix(number[10][20] input) {"
            "number[10][20] result;"
            "result[2][5] = input[1][3];"
            "return null;"
        "}"
        "function void main() {"
            "Point[3][4] grid;"
            "number[5][6][7] tensor;"
            "grid[1][2].x = 10;"
            "tensor[1][2][3] = 42;"
        "}";


    Parser parser;
    parser_init(&parser, source);

    ASTNode *node = parser_parse_program(&parser);

    if (node != NULL) {
        ast_print(node, 0);
        ast_free(node);
    }

    return 0;
}