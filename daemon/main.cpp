#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cstdint>
#include <WinSock2.h>
#include <string>
#include <Shlwapi.h>
#include <vector>
#include "../common/json.h"
#include "../common/defs.h"
#include "../common/db.h"
#include "../common/path.h"
#include "reader.h"

#define MAX_START_TIME  1610668800

//void serviceMain (int argc, char** argv);
int startService ();
int stopService ();
int installService ();
int removeService ();
void runService ();
void log (char *msg);

pollerContext *runnerCtx = 0;

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
        "\t-e:timestamp\tinterval end\n\n"
        "Service control parameters:\n"
        "\t-i\tinstall service\n"
        "\t-u\tuninstall service\n"
        "\t-s\tstart service\n"
        "\t-o\tstop service\n"
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

bool manageService (char command) {
    switch (command) {
        case 'i': installService (); return true;
        case 'u': removeService (); return true;
        case 's': startService (); return true;
        case 'o': stopService (); return true;
        default: return false;
    }
}

void stopPoller () {
    if (runnerCtx) {
        runnerCtx->keepRunning = false;
    
        /*if (runnerCtx->runner->joinable ()) runnerCtx->runner->join ();

        stopAllReaders ();*/
    }
}

void doNormalWork () {
    config cfg;
    database db (cfg, getPath ());
    std::string path = getPath ();

    log ("enter doNormalWork");

    path += "cfg.dat";
    cfg.cfgFile = path.c_str ();

    parseCfgFile (cfg);

    log ("staring poller");

    runnerCtx = startPoller (cfg);

    log ("poller started, starting readers");

    startAllReaders (cfg, db);
 
    log ("readers started, wait runner threads");
    if (runnerCtx->runner->joinable ()) runnerCtx->runner->join ();

    stopAllReaders ();

    log ("exit doNormalWork");
}

int main (int argCount, char *args []) {
    if (argCount == 1) {
        runService ();
    } else if (args [1][0] == '-' || args [1][0] == '/') {
        if (manageService (tolower (args [1][1]))) exit (0);

        config cfg;
        database db (cfg, getPath ());

        printf ("OBL Daemon v1.0\n");

        if (time (0) > MAX_START_TIME) {
            printf ("The license has been expired.\n");
            exit (0);
        }

        parseParams (argCount, args, cfg);
        
        auto runnerCtx = startPoller (cfg);

        startAllReaders (cfg, db);
    
        if (runnerCtx->runner->joinable ()) runnerCtx->runner->join ();

        stopAllReaders ();
    }
    
    exit (0);
}