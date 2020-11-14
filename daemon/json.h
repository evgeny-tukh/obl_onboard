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
    };

    struct stringNode: node {
        std::string value;

        stringNode (): node (nodeType::string) {}
        stringNode (const char *src): node (nodeType::string), value (src) {}

        virtual void *get () { return (void *) value.c_str (); }
        inline const char *getValue () { return value.c_str (); }
    };

    struct numberNode: node {
        double value;

        numberNode (): node (nodeType::number) {}
        numberNode (const char *src): node (nodeType::number), value (atof (src)) {}

        virtual void *get () { return (void *) & value; }
        inline double getValue () { return value; }
    };

    struct arrayNode: node {
        std::vector<node *> value;

        arrayNode (): node (nodeType::array) {}

        virtual void *get () { return (void *) & value; }

        inline size_t size () {
            return value.size ();
        }

        inline void add (node *val) {
            value.push_back (val);
        }

        inline node *operator [] (size_t index) {
            return index < value.size () ? value [index] : 0;
        }

        inline auto begin () { return value.begin (); }
        inline auto end () { return value.end (); }
    };

    struct hashNode: node {
        std::map<char *, node *> value;

        hashNode (): node (nodeType::hash) {}
        virtual ~hashNode () {
            for (auto& item: value) {
                if (item.first) free (item.first);
            }
        }

        virtual void *get () { return (void *) & value; }

        inline void add (char *key, node *val) {
            value.insert (value.end (), std::pair<char *, node *> (_strdup (key), val));
        }

        inline node *operator [] (char *key) {
            for (auto& item: value) {
                auto result = strcmp (item.first, key);
                
                if (result == 0) return item.second;
            }

            return 0;
        }

        inline auto begin () { return value.begin (); }
        inline auto end () { return value.end (); }
    };

    node *parse (char *sourceString, int& nextChar);
    void removeWhiteSpaces (char *source, std::string& result);
}
