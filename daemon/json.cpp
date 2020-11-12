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

    struct node {
        nodeType type;

        node (nodeType _type = nodeType::null) : type (_type) {}

        virtual void *get () { return 0; }
        virtual void parse (char *) {}
    };

    struct stringNode: node {
        std::string value;

        stringNode (): node (nodeType::string) {}
        stringNode (const char *src): node (nodeType::string), value (src) {}

        virtual void *get () { return (void *) value.c_str (); }
        virtual void parse (char *) {}
    };

    struct numberNode: node {
        double value;

        numberNode (): node (nodeType::number) {}
        numberNode (const char *src): node (nodeType::number), value (atof (src)) {}

        virtual void *get () { return (void *) & value; }
        virtual void parse (char *) {}
    };

    struct arrayNode: node {
        std::vector<node *> value;

        arrayNode (): node (nodeType::array) {}

        virtual void *get () { return (void *) & value; }
        virtual void parse (char *) {}

        void add (node *val) {
            value.push_back (val);
        }

        node *operator [] (size_t index) { return value [index]; }
    };

    struct hashNode: node {
        std::map<char *, node *> value;

        hashNode (): node (nodeType::hash) {}

        virtual void *get () { return (void *) & value; }
        virtual void parse (char *);

        void add (char *key, node *val) {
            value.insert (value.end (), std::pair<char *, node *> (key, val));
        }

        node *operator [] (char *key) { return value [key]; }
    };

    char *findEndBrace (char *brace);
    void split (char *source, char separator, std::vector<std::string>& parts);
    void parseKvp (char *source, std::string& key, std::string& value);
    void removeWhitespaces (char *source, std::string& result);

    node *parse (char *source, char *&next);
    char *extractLiteral (char *start, std::string& extraction);
    char *extractNumber (char *start, std::string& extraction);
    char *extractHash (char *start, std::string& extraction);
    char *extractArray (char *start, std::string& extraction);
}

void json::parseKvp (char *source, std::string& key, std::string& value) {
    auto colon = strchr (source, ':');

    key.clear ();
    value.clear ();

    if (colon) {
        key.insert (key.begin (), source, colon);
        value.insert (value.begin (), colon + 1, colon + strlen (colon));
    }
}

void json::split (char *source, char separator, std::vector<std::string>& parts) {
    parts.clear ();

    char *start = source;
    char *end;
    
    auto addPart = [&parts] (char *from, char *to) {
        std::string part;

        part.insert (part.begin (), from, to);
        parts.push_back (part);
    };

    do {
        end = strchr (start + 1, separator);

        if (end) {
            addPart (start, end);

            start = end + 1;
        } else {
            addPart (start, start + strlen (start));

            start = 0;
        }
    } while (start && *start);
}

char *json::findEndBrace (char *brace) {
    char beginBrace = *brace, endBrace;

    switch (*brace) {
        case '{': endBrace = '}'; break;
        case '[': endBrace = ']'; break;
        default: return 0;
    }

    int count = 1;

    for (auto chr = brace + 1; *chr; ++ chr) {
        if (*chr == beginBrace) {
            ++ count;
        } else if (*chr == endBrace) {
            -- count;

            if (count == 0) return chr;
        }
    }

    return 0;
};

char *json::extractHash (char *start, std::string& extraction) {
    char *result = 0;

    extraction.clear ();

    if (*start = '{') {
        auto end = findEndBrace (start);

        if (end) {
            result = end + 1;

            extraction.insert (extraction.begin (), start + 1, end - 1);
        }
    }

    return result;
}

char *json::extractArray (char *start, std::string& extraction) {
    char *result = 0;

    extraction.clear ();

    if (*start = '[') {
        auto end = findEndBrace (start);

        if (end) {
            result = end + 1;

            extraction.insert (extraction.begin (), start + 1, end - 1);
        }
    }

    return result;
}

char *json::extractLiteral (char *start, std::string& extraction) {
    char *result = 0;

    extraction.clear ();

    if (*start == '"') {
        char *chr;

        for (chr = start + 1; *chr && *chr != '"'; extraction += *(chr ++));

        if (*chr == '"') ++ chr;

        result = chr;
    }

    return result;
}

char *json::extractNumber (char *start, std::string& extraction) {
    char *result = 0;

    extraction.clear ();

    if (*start == '-') {
        extraction += *(start ++);
    }

    if (isdigit (*start)) {
        char *chr;
        bool dotPassed = false;

        for (chr = start; *chr && *chr != ','; extraction += *(chr ++)) {
            switch (*chr) {
                case '.': {
                    if (dotPassed) exit (0);
                    
                    dotPassed = true; break;
                }
                default: {
                    if (!isdigit (*chr)) exit (0);
                }
            }
        }

        if (*chr == ',') ++ chr;

        result = chr;
    }

    return result;
}

void json::hashNode::parse (char *source) {
    char key [200];

    while (*source == '"') {
        auto nextChar = extractLiteral (source, key, sizeof (key));
        auto count = nextChar - source;

        if (source [count] != ':') exit (0);

        size_t next;

        value [key] = json::parse (source + (++count), next);

        if (source [next] != ',' && source [next] != '\0') exit (0);

        source += next + 1;
    }
}

void json::removeWhitespaces (char *source, std::string& result) {
    result.clear ();

    bool inQuote = false;

    for (char *org = source; *org; ++ org) {
        switch (*org) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                if (inQuote) result += *org;
                break;

            case '\"':
                inQuote = !inQuote;

            default:
                result += *org;
        }
    }
}

json::node *json::parse (char *sourceString, char *& next) {
    std::string source;

    removeWhitespaces (sourceString, source);

    node *result = 0;

    switch (source [0]) {
        case '{': {
            std::string hashData;
            std::vector<std::string> parts;
            auto item = new json::hashNode ();

            next = extractHash ((char *) source.c_str (), hashData);

            split ((char *) hashData.c_str (), ',', parts);

            for (auto& part: parts) {
                std::string key, value;
                char *dummy;

                parseKvp ((char *) part.c_str (), key, value);

                auto node = parse ((char *) value.c_str (), dummy);

                item->add ((char *) key.c_str (), node);
            }

            break;
        }
        case '[': {
            std::string arrData;
            std::vector<std::string> parts;
            auto item = new json::arrayNode ();

            next = extractArray ((char *) source.c_str (), arrData);

            split ((char *) arrData.c_str (), ',', parts);

            for (auto& part: parts) {
                char *dummy;

                auto node = parse ((char *) part.c_str (), dummy);

                item->add (node);
            }

            break;
        }
        case '"': {
            std::string value;

            extractLiteral ((char *) source.c_str (), value);

            result = new json::stringNode (value.c_str ()); break;
        }
        default: {
            if (isdigit (source [0]) || source [0] == '-') {
                std::string value;

                json::extractNumber ((char *) source.c_str (), value);

                result = new json::numberNode ((char *) value.c_str ()); break;
            } else {
                result = new node ();
            }
        }
    }

    return result;
}
