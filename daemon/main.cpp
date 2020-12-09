#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cstdint>
#include <WinSock2.h>
#include <string>
#include <vector>
#include "../common/json.h"
#include "../common/defs.h"
#include "../common/db.h"
#include "reader.h"

void showHelp () {
    printf (
        "USAGE:\n"
        "daemon [options]\n\n"
        "where options are:\n"
        "\t-?\t\tshow help\n"
        "\t-p:port\t\tset port\n"
        "\t-h:host\t\tset host\n"
        "\t-a:path\t\tset host path\n"
        "\t-c:cfgfile\tset config file\n"
        "\t-q\t\tquery data values; might be used in together with -b and -e; otherwise recent data will be received\n"
        "\t-b:timestamp\tinterval begin\n"
        "\t-e:timestamp\tinterval end\n"
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
                case 'b': {
                    if (arg [2] == ':') {
                        cfg.begin = atol (arg + 3);
                    } else {
                        invalidArgMsg (arg);
                    }
                    break;
                }
                case 'e': {
                    if (arg [2] == ':') {
                        cfg.end = atol (arg + 3);
                    } else {
                        invalidArgMsg (arg);
                    }
                    break;
                }
                case 'p': {
                    if (arg [2] == ':') {
                        cfg.port = atoi (arg + 3);
                    } else {
                        invalidArgMsg (arg);
                    }
                    break;
                }
                case 'a': {
                    if (arg [2] == ':') {
                        cfg.path = arg + 3;
                    } else {
                        invalidArgMsg (arg);
                    }
                    break;
                }
                case 'h': {
                    if (arg [2] == ':') {
                        cfg.host = arg + 3;
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
                case 'q': {
                    cfg.queryData = true; break;
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
    database db (cfg);

    printf ("OBL Daemon v1.0\n");

    parseParams (argCount, args, cfg);
    
    auto runnerCtx = startPoller (cfg);

    startAllReaders (cfg);
 
    if (runnerCtx->runner->joinable ()) runnerCtx->runner->join ();

    stopAllReaders ();
    
    exit (0);
}