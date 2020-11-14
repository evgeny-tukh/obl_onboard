#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cstdint>
#include <WinSock2.h>
#include <string>
#include "json.h"

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
        std::string source;
        json::removeWhiteSpaces (buffer, source);
        
        auto json = json::parse ((char *) source.c_str (), next);
        json::hashNode *root = (json::hashNode *) json;
        json::hashNode *server = (json::hashNode *) (*root) ["server"];
        json::numberNode *port = (json::numberNode *) (*server) ["port"];
        json::stringNode *host = (json::stringNode *) (*server) ["host"];

        printf ("port: %.f; host: %s\n", port->getValue (), host->getValue ());

        json::arrayNode *values = (json::arrayNode *) (*root) ["values"];

        for (auto curVal = values->begin (); curVal != values->end (); ++ curVal) {
            if ((*curVal)->type == json::nodeType::null) {
                printf ("Null value\n");
            } else {
                json::numberNode *val = (json::numberNode *) (*curVal);

                printf ("Value %.f\n", val->getValue ());
            }
        }

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

    exit (0);
}