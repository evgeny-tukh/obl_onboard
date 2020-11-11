#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

namespace json {

    enum nodeType {
        null = 0,
        number,
        string,
        array,
        hash,
    };

    struct node;
    struct numberNode;
    struct hashNode;
    struct arrayNode;
    struct stringNode;

    node *parse (char *source, size_t& next);
}
