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

        virtual void *get () { return (void *) value.c_str (); }
        virtual void parse (char *) {}
    };

    struct numberNode: node {
        double value;

        numberNode (): node (nodeType::number) {}

        virtual void *get () { return (void *) & value; }
        virtual void parse (char *) {}
    };

    struct arrayNode: node {
        std::vector<node *> value;

        arrayNode (): node (nodeType::array) {}

        virtual void *get () { return (void *) & value; }
        virtual void parse (char *) {}

        node *operator [] (size_t index) { return value [index]; }
    };

    struct hashNode: node {
        std::map<char *, node *> value;

        hashNode (): node (nodeType::hash) {}

        virtual void *get () { return (void *) & value; }
        virtual void parse (char *);

        node *operator [] (char *key) { return value [key]; }
    };

    node *parse (char *source, size_t& next);
}

void json::hashNode::parse (char *source) {
    char key [200];

    while (*source == '"') {
        auto count = 1;

        for (auto i = 1; source [i] != '"'; ++ i, ++ count) {
            if (source [i] == '\0') exit (0);

            key [i-1] = source [i];
            key [i] = '\0';
        }

        if (source [++count] != ':') exit (0);

        size_t next;

        value [key] = json::parse (source + count, next);

        if (source [next] != ',' && source [next] != '\0') exit (0);

        source += next + 1;
    }
}

json::node *json::parse (char *source, size_t& next) {
    char *copy = (char *) malloc (strlen (source));

    // remove whitespaces
    bool inQuote = false;

    for (char *org = source, *dest = copy; *org; ++ org) {
        switch (*org) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                if (inQuote) *(dest ++) = *org;
                break;

            case '\"':
                inQuote = !inQuote;

            default:
                *(dest ++) = *org;
        }

        *dest = '\0';
    }

    auto findEndBrace = [copy] (char *bracePtr) {
        char beginBrace = *bracePtr, endBrace;

        switch (*bracePtr) {
            case '{': endBrace = '}'; break;
            case '[': endBrace = ']'; break;
            default: exit (0);
        }

        int count = 1;

        for (auto chr = bracePtr + 1; *chr; ++ chr) {
            if (*chr == beginBrace) {
                ++ count;
            } else if (*chr == endBrace) {
                -- count;

                if (count == 0) return chr;
            }
        }

        return (char *) 0;
    };

    auto extractNodeText = [] (char *start, char *end) {
        auto size = end - start - 1;
        char *nodeText = (char *) malloc (size + 1);

        memcpy (nodeText, start + 1, size - 1);

        nodeText [size-1] = '\0';

        return nodeText;
    };

    node *result = 0;

    for (auto chr = copy; *chr; ++ chr) {
        switch (*chr) {
            case '{': {
                auto end = findEndBrace (chr);
                char *nodeText = extractNodeText (chr, end);
                auto newNode = new json::hashNode ();

                newNode->parse (nodeText); 

                result = newNode;
                chr = end;
                next = (end - chr + 1);
                
                break;
            }
            case '[': {
                auto end = findEndBrace (chr);
                char *nodeText = extractNodeText (chr, end);
                auto newNode = new json::arrayNode ();

                newNode->parse (nodeText); 

                result = newNode;
                chr = end;
                next = (end - chr + 1);
                
                break;
            }
            default: {
                result = new node ();
            }
        }
    }

    free (copy);

    return result;
}
