#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cstdint>
#include <WinSock2.h>
#include <string>
#include <vector>
#include "json.h"
#include "defs.h"
#include "db.h"

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
        
        auto json = json::parse (/*(char *) source.c_str ()*/buffer, next);
        json::hashNode *root = (json::hashNode *) json;
        json::hashNode *server = (json::hashNode *) (*root) ["server"];
        json::numberNode *port = (json::numberNode *) (*server) ["port"];
        json::stringNode *host = (json::stringNode *) (*server) ["host"];
        json::stringNode *path = (json::stringNode *) (*server) ["path"];
        json::hashNode *shipInfo = (json::hashNode *) (*root) ["vessel"];
        json::arrayNode *tanks = (json::arrayNode *) (*root) ["tanks"];
        json::hashNode *settings = (json::hashNode *) (*root) ["settings"];
        json::arrayNode *params = (json::arrayNode *) (*root) ["params"];
        json::arrayNode *paramGroups = (json::arrayNode *) (*root) ["paramGroups"];
        json::hashNode *columnMap = (json::hashNode *) (*root) ["columnMap"];

        if (host) cfg.host = host->getValue ();
        if (path) cfg.path = path->getValue ();
        if (port) cfg.port = (uint16_t) port->getValue ();
        if (shipInfo) {
            auto& ship = (*shipInfo);
            json::stringNode *master = (json::stringNode *) ship ["master"];
            json::stringNode *cheng = (json::stringNode *) ship ["cheng"];
            json::stringNode *name = (json::stringNode *) ship ["name"];
            json::numberNode *mmsi = (json::numberNode *) ship ["mmsi"];
            json::numberNode *imo = (json::numberNode *) ship ["imo"];
            json::numberNode *mtID = (json::numberNode *) ship ["mt_id"];

            if (master) cfg.shipInfo.master = master->getValue ();
            if (cheng) cfg.shipInfo.cheng = cheng->getValue ();
            if (name) cfg.shipInfo.name = name->getValue ();
            if (mmsi) cfg.shipInfo.mmsi = mmsi->getValue ();
            if (imo) cfg.shipInfo.imo = imo->getValue ();
            if (mtID) cfg.shipInfo.mtID = mtID->getValue ();
        }
        if (tanks) {
            for (auto i = 0; i < tanks->size (); ++ i) {
                json::hashNode *tank = (json::hashNode *) (*tanks) [i];

                if (tank) {
                    json::numberNode *id = (json::numberNode *) (*tank) ["id"];
                    json::stringNode *name = (json::stringNode *) (*tank) ["name"];
                    json::stringNode *type = (json::stringNode *) (*tank) ["type"];
                    json::numberNode *volume = (json::numberNode *) (*tank) ["volume"];

                    if (id, name && type && volume) {
                        cfg.tanks.emplace_back ((uint16_t) id->getValue (), name->getValue (), type->getValue (), volume->getValue ());
                    }
                }
            }
        }
        if (settings) {
            json::numberNode *pollingInterval = (json::numberNode *) (*settings) ["pollingInterval"];
            json::numberNode *timezone = (json::numberNode *) (*settings) ["timezone"];

            if (pollingInterval) cfg.pollingInterval = (time_t) pollingInterval->getValue ();
            if (timezone) cfg.timezone = (float) timezone->getValue ();
        }
        if (params) {
            for (auto i = 0; i < params->size (); ++ i) {
                json::hashNode *parameter = (json::hashNode *) (*params) [i];

                if (parameter) {
                    json::numberNode *id = (json::numberNode *) (*parameter) ["id"];
                    json::stringNode *key = (json::stringNode *) (*parameter) ["key"];
                    json::stringNode *name = (json::stringNode *) (*parameter) ["name"];
                    json::numberNode *isNumber = (json::numberNode *) (*parameter) ["isNumber"];
                    json::numberNode *multiplier = (json::numberNode *) (*parameter) ["multiplier"];
                    json::numberNode *group = (json::numberNode *) (*parameter) ["group"];

                    auto& newParam = cfg.params.emplace (
                        (uint8_t) id->getValue (),
                        param (
                            (uint8_t) id->getValue (),
                            key->getValue (),
                            name->getValue (),
                            (uint8_t) multiplier->getValue (),
                            (uint8_t) isNumber->getValue (),
                            (uint8_t) group->getValue ()
                        )
                    ).first->second;
                }
            }
        }
        if (paramGroups) {
            for (auto i = 0; i < paramGroups->size (); ++ i) {
                json::hashNode *parameterGroup = (json::hashNode *) (*paramGroups) [i];

                if (parameterGroup) {
                    json::numberNode *id = (json::numberNode *) (*parameterGroup) ["id"];
                    json::stringNode *key = (json::stringNode *) (*parameterGroup) ["key"];
                    json::stringNode *name = (json::stringNode *) (*parameterGroup) ["name"];

                    cfg.paramGroups.emplace (
                        (uint8_t) id->getValue (),
                        paramGroup (
                            (uint8_t) id->getValue (),
                            key->getValue (),
                            name->getValue ()
                        )
                    );
                }
            }
        }
        if (columnMap) {
            for (auto iter = columnMap->begin (); iter != columnMap->end (); ++ iter) {
                json::numberNode *column = (json::numberNode *) iter->second;

                cfg.columnMap.emplace (iter->first, (uint8_t) column->getValue ());
            }
        }

        printf ("port: %.f; host: %s\n", port->getValue (), host->getValue ());

        /*json::arrayNode *values = (json::arrayNode *) (*root) ["values"];

        json::valueKey key;
        uint16_t level = 0;
        json::walkThrough (json, showValue, key, level);

        next = 0;
        values->setAt (2, json::parse ("{\"val1\":\"abc\",\"val2\":5,\"val3\":{\"sv1\":11,\"sv2\":22}}", next));

        level = 0;
        key.arrayIndex = json::valueKey::noIndex;
        key.hashKey.clear ();
        json::walkThrough (json, showValue, key, level);
        //json::walkThrough (values, showValue, key);

        printf ("\n\n%s\n", json->serialize ().c_str ());*/

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
    database db;

    printf ("OBL Daemon v1.0\n");

    parseParams (argCount, args, cfg);
    
    auto runnerCtx = startPoller (cfg);
 
    if (runnerCtx->runner->joinable ()) runnerCtx->runner->join ();

    exit (0);
}