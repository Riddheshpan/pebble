#include <stdio.h>

#include "parser.h"

int main(void)
{
    const char *source =
        "import \"math.peb\";"

        "struct Person {"
            "string name;"
            "number age;"
            "number[10] scores;"
        "}"

        "enum Color { RED, GREEN, BLUE }"

        "function number add(number a, number b) {"
            "number result a + b;"
            "if (result > 10) {"
                "print result;"
            "} else {"
                "print 0;"
            "}"
            "return result;"
        "}"

        "function void greet(string name) {"
            "print \"Hello, \";"
            "print name;"
        "}"

        "function number main() {"
            "number i 0;"
            "while (i < 10) {"
                "if (i == 5) {"
                    "break;"
                "}"
                "i = i + 1;"
            "}"
            "repeat (3) {"
                "print i;"
            "}"
            "for (number j 0; j < 3; j = j + 1) {"
                "print j;"
            "}"
            "return 0;"
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