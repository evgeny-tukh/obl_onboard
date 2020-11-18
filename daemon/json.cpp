#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

#include "json.h"

namespace json {
    node *parse (char *stream, int& offset);

    bool extractLiteral (char *stream, std::string& extraction, int& offset);
    node *extractLiteral (char *stream, int& offset);
    node *extractNumber (char *stream, int& offset);
    node *extractHash (char *stream, int& offset);
    node *extractArray (char *stream, int& offset);
}

void json::removeWhiteSpaces (char *source, std::string& result) {
    bool insideString = false;

    result.clear ();

    for (auto chr = source; *chr; ++ chr) {
        if (*chr == '"') insideString = !insideString;
        
        switch (*chr) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                if (!insideString) break;

            default:
                result += *chr;
        }
    }
}

bool json::extractLiteral (char *stream, std::string& extraction, int& offset) {
    extraction.clear ();

    if (stream [offset] != '"') return false;

    for (auto i = offset + 1; stream [i] && stream [i] != '"'; ++ i) extraction += stream [i];

    offset += extraction.length () + 2;

    return true;
}

json::node *json::extractLiteral (char *stream, int& offset) {
    std::string extraction;

    if (!extractLiteral (stream, extraction, offset)) return 0;

    return new stringNode (extraction.c_str ());
}

json::node *json::extractNumber (char *stream, int& offset) {
    if (!isdigit (stream [offset]) && stream [offset] != '-') return 0;

    std::string extraction;
    bool dotPassed = false;

    if (stream [offset] == '-') {
        extraction += '-';

        ++ offset;
    }

    for (auto i = offset; isdigit (stream [i]) || stream [i] == '.'; ++ i) {
        if (stream [i] == '.') {
            if (dotPassed) return 0;

            dotPassed = true;
        }

        extraction += stream [i];
    }

    offset += extraction.length ();

    return new numberNode (extraction.c_str ());
}

json::node *json::parse (char *stream, int& offset) {
    json::node *item;
    std::string streamHolder;

    removeWhiteSpaces (stream, streamHolder);

    stream = (char *) streamHolder.c_str ();

    switch (stream [offset]) {
        case '{': {
            item = extractHash (stream, offset);

            if (!item) return 0;

            break;
        }
        case '[': {
            item = extractArray (stream, offset);

            if (!item) return 0;

            break;
        }
        case '"': {
            item = extractLiteral (stream, offset);

            if (!item) return 0;

            break;
        }
        default: {
            if (isdigit (stream [offset]) || stream [offset] == '.') {
                item = extractNumber (stream, offset);
            } else if (memcmp (stream + offset, "null", 4) == 0) {
                item = new node ();
                offset += 4;
            } else {
                item = 0;
            }

            break;
        }
    }

    return item;
}

json::node *json::extractHash (char *stream, int& offset) {
    hashNode *result = new hashNode ();
    
    if (stream [offset] != '{') return  0;

    ++ offset;

    while (stream [offset] == '"') {
        std::string key;

        if (!extractLiteral (stream, key, offset)) return 0;
        if (stream [offset] != ':') return 0;

        ++ offset;

        json::node *item = parse (stream, offset);

        if (!item) return 0;

        result->add ((char *) key.c_str (), item);

        if (stream [offset] == ',') ++ offset;
    }

    if (stream [offset] != '}') return 0;

    ++ offset;

    return result;
}

json::node *json::extractArray (char *stream, int& offset) {
    arrayNode *result = new arrayNode ();
    
    if (stream [offset] != '[') return  0;

    ++ offset;

    while (stream [offset] != ']') {
        json::node *item = parse (stream, offset);

        if (!item) return 0;

        result->add (item);

        if (stream [offset] == ',') ++ offset;
    }

    if (stream [offset] != ']') return 0;

    ++ offset;

    return result;
}

void json::getValue (node *_node, nodeValue& value) {
    switch ((*_node).type) {
        case nodeType::number: {
            value.numericValue = ((numberNode *)_node)->getValue (); break;
        }
        case nodeType::string: {
            value.stringValue = ((stringNode *)_node)->getValue (); break;
        }
        case nodeType::hash: {
            hashNode *hash = (hashNode *) _node;
            value.hashValue.insert ((*hash).begin (), (*hash).end ()); break;
        }
        case nodeType::array: {
            arrayNode *array = (arrayNode *) _node;
            value.arrayValue.insert (value.arrayValue.begin (), (*array).begin (), (*array).end ()); break;
        }
    }
}