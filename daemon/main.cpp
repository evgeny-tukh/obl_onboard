#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cstdint>
#include <WinSock2.h>
#include <string>
#include "json.h"
#include "req_mgr.h"

struct config {
    in_addr host;
    uint16_t port;
    std::string cfgFile;

    config () : port (3500) {
        host.S_un.S_addr = INADDR_LOOPBACK;
    }
};

void parseCfgFile (config& cfg) {
    FILE *cfgFile = fopen (cfg.cfgFile.c_str (), "rb");

    if (cfgFile) {
        fseek (cfgFile, 0, SEEK_END);

        auto size = ftell (cfgFile);

        fseek (cfgFile, 0, SEEK_SET);

        char *buffer = (char *) malloc (size + 1);

        memset (buffer, 0, size + 1);

        fread (buffer, sizeof (char), size, cfgFile);

        int next = 0;
        //std::string source;
        //json::removeWhiteSpaces (buffer, source);
        
        auto json = json::parse (/*(char *) source.c_str ()*/buffer, next);
        json::hashNode *root = (json::hashNode *) json;
        json::hashNode *server = (json::hashNode *) (*root) ["server"];
        json::numberNode *port = (json::numberNode *) (*server) ["port"];
        json::stringNode *host = (json::stringNode *) (*server) ["host"];

        printf ("port: %.f; host: %s\n", port->getValue (), host->getValue ());

        json::arrayNode *values = (json::arrayNode *) (*root) ["values"];

        auto showValue = [] (json::node *_node, json::nodeValue& nodeVal, json::valueKey& key, uint16_t level) {
            printf ("Level %d ", level);

            if (key.arrayIndex != json::valueKey::noIndex) {
                printf ("[%zd] ", key.arrayIndex);
            } else if (!key.hashKey.empty ()) {
                printf ("[%s] ", key.hashKey.c_str ());
            }

            if (!_node) {
                printf ("Missing value\n");
            } else {
                switch (_node->type) {
                    case json::nodeType::null:
                        printf ("Null value\n"); break;
                    case json::nodeType::number:
                        printf ("%f\n", nodeVal.numericValue); break;
                    case json::nodeType::string:
                        printf ("%s\n", nodeVal.stringValue.c_str ()); break;
                    case json::nodeType::array:
                        printf ("array\n"); break;
                    case json::nodeType::hash:
                        printf ("hash\n"); break;
                }
            }
        };

        json::valueKey key;
        uint16_t level = 0;
        json::walkThrough (json, showValue, key, level);
        //json::walkThrough (values, showValue, key);

        next = 0;
        values->setAt (2, json::parse ("{\"val1\":\"abc\",\"val2\":5,\"val3\":{\"sv1\":11,\"sv2\":22}}", next));

        level = 0;
        key.arrayIndex = json::valueKey::noIndex;
        key.hashKey.clear ();
        json::walkThrough (json, showValue, key, level);
        //json::walkThrough (values, showValue, key);

        printf ("\n\n%s\n", json->serialize ().c_str ());

        free (buffer);
        fclose (cfgFile);
    }
}

void showHelp () {
    printf (
        "USAGE:\n"
        "daemon [options]\n\n"
        "where options are:\n"
        "\t-?\t\tshow help\n"
        "\t-p:port\t\tset port\n"
        "\t-h:host\t\tset host\n"
        "\t-c:cfgfile\tset config file\n"
    );
    
    exit (0);
}

void invalidArgMsg (char *arg) {
    printf ("Inavlid argument %s\n", arg);
    exit (0);
}

void parseParams (int argCount, char *args [], config& cfg) {
    for (auto i = 1; i < argCount; ++ i) {
        auto arg = args [i];

        if (arg [0] == '-' || arg [0] == '/') {
            switch (tolower (arg [1])) {
                case '?': {
                    showHelp ();
                }
                case 'p': {
                    if (arg [2] == ':') {
                        cfg.port = atoi (arg + 3);
                    } else {
                        invalidArgMsg (arg);
                    }
                    break;
                }
                case 'h': {
                    if (arg [2] == ':') {
                        cfg.host.S_un.S_addr = inet_addr (arg + 3);
                    } else {
                        invalidArgMsg (arg);
                    }
                    break;
                }
                case 'c': {
                    if (arg [2] == ':') {
                        cfg.cfgFile = arg + 3;
                    } else {
                        invalidArgMsg (arg);
                    }
                    break;
                }
            }
        } else {
            showHelp ();
        }
    }

    if (cfg.cfgFile.length () > 0) parseCfgFile (cfg);
}

int main (int argCount, char *args []) {
    config cfg;

    printf ("OBL Daemon v1.0\n");

    parseParams (argCount, args, cfg);

    reqManager mgr (3500, "localhost");

    if (mgr.open ()) {
        char buffer [10000];

        if (mgr.sendRequest ("config", buffer, sizeof (buffer)))
            printf ("%s\n\n", buffer);
        if (mgr.sendRequest ("channels", buffer, sizeof (buffer)))
            printf ("%s\n\n", buffer);
    }

    exit (0);
}