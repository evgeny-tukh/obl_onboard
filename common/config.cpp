#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <cstdint>
#include <string>
#include <vector>
#include "json.h"
#include <Windows.h>
#include "defs.h"

draftData::draftData (config& cfg): fore (cfg.shipInfo.normalDraft), aft (cfg.shipInfo.normalDraft) {}

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
        int elemId;
        
        auto json = json::parse (/*(char *) source.c_str ()*/buffer, next);
        json::hashNode *root = (json::hashNode *) json;
        json::hashNode *server = (json::hashNode *) (*root) ["server"];
        json::numberNode *port = (json::numberNode *) (*server) ["port"];
        json::stringNode *host = (json::stringNode *) (*server) ["host"];
        json::stringNode *path = (json::stringNode *) (*server) ["path"];
        json::hashNode *shipInfo = (json::hashNode *) (*root) ["vessel"];
        json::arrayNode *tanks = (json::arrayNode *) (*root) ["tanks"];
        json::arrayNode *fuelMeters = (json::arrayNode *) (*root) ["fuelMeters"];
        json::hashNode *settings = (json::hashNode *) (*root) ["settings"];
        json::arrayNode *params = (json::arrayNode *) (*root) ["params"];
        json::arrayNode *paramGroups = (json::arrayNode *) (*root) ["paramGroups"];
        json::hashNode *columnMap = (json::hashNode *) (*root) ["columnMap"];
        json::hashNode *repCfg = (json::hashNode *) (*root) ["report"];
        json::arrayNode *sensors = (json::arrayNode *) (*root) ["sensors"];
        json::stringNode *draftAft = (json::stringNode *) (*root) ["draftAftChannel"];
        json::stringNode *draftFore = (json::stringNode *) (*root) ["draftForeChannel"];
        json::arrayNode *layout = (json::arrayNode *) (*root) ["layout"];

        if (host != json::nothing) cfg.host = host->getValue ();
        if (path != json::nothing) cfg.path = path->getValue ();
        if (port != json::nothing) cfg.port = (uint16_t) port->getValue ();
        if (repCfg != json::nothing) {
            json::stringNode *templ = (json::stringNode *) (*repCfg) ["template"];
            json::stringNode *dataExport = (json::stringNode *) (*repCfg) ["export"];

            if (templ != json::nothing) cfg.repCfg.templatePath = templ->getValue ();
            if (dataExport != json::nothing) cfg.repCfg.exportPath = dataExport->getValue ();
        }
        if (shipInfo != json::nothing) {
            auto& ship = (*shipInfo);
            json::stringNode *master = (json::stringNode *) ship ["master"];
            json::stringNode *cheng = (json::stringNode *) ship ["cheng"];
            json::stringNode *name = (json::stringNode *) ship ["name"];
            json::numberNode *mmsi = (json::numberNode *) ship ["mmsi"];
            json::numberNode *imo = (json::numberNode *) ship ["imo"];
            json::numberNode *mtID = (json::numberNode *) ship ["mt_id"];
            json::numberNode *draft = (json::numberNode *) ship ["normalDraft"];

            if (master != json::nothing) cfg.shipInfo.master = master->getValue ();
            if (cheng != json::nothing) cfg.shipInfo.cheng = cheng->getValue ();
            if (name != json::nothing) cfg.shipInfo.name = name->getValue ();
            if (mmsi != json::nothing) cfg.shipInfo.mmsi = mmsi->getValue ();
            if (imo != json::nothing) cfg.shipInfo.imo = imo->getValue ();
            if (draft != json::nothing) cfg.shipInfo.normalDraft = draft->getValue ();
        }
        if (tanks != json::nothing) {
            for (auto i = 0; i < tanks->size (); ++ i) {
                json::hashNode *tank = (json::hashNode *) (*tanks) [i];

                if (tank != json::nothing) {
                    json::numberNode *id = (json::numberNode *) (*tank) ["id"];
                    json::stringNode *name = (json::stringNode *) (*tank) ["name"];
                    json::stringNode *type = (json::stringNode *) (*tank) ["type"];
                    json::numberNode *volume = (json::numberNode *) (*tank) ["volume"];
                    json::stringNode *side = (json::stringNode *) (*tank) ["side"];

                    if (id  != json::nothing && name != json::nothing && type != json::nothing && volume != json::nothing) {
                        cfg.tanks.emplace_back ((uint16_t) id->getValue (), name->getValue (), type->getValue (), (float) volume->getValue (), side->getValue ());
                    }
                }
            }
        }
        if (fuelMeters != json::nothing) {
            for (auto i = 0; i < fuelMeters->size (); ++ i) {
                json::hashNode *fuelMeter = (json::hashNode *) (*fuelMeters) [i];

                if (fuelMeter != json::nothing) {
                    json::numberNode *id = (json::numberNode *) (*fuelMeter) ["id"];
                    json::stringNode *name = (json::stringNode *) (*fuelMeter) ["name"];
                    json::stringNode *type = (json::stringNode *) (*fuelMeter) ["type"];

                    if (id != json::nothing && name != json::nothing && type != json::nothing) {
                        cfg.fuelMeters.emplace_back ((uint16_t) id->getValue (), name->getValue (), type->getValue ());
                    }
                }
            }
        }
        if (settings != json::nothing) {
            json::numberNode *pollingInterval = (json::numberNode *) (*settings) ["pollingInterval"];
            json::numberNode *timeout = (json::numberNode *) (*settings) ["timeout"];
            json::numberNode *logbookPeriod = (json::numberNode *) (*settings) ["logbookPeriod"];
            json::numberNode *timezone = (json::numberNode *) (*settings) ["timezone"];
            json::stringNode *newDataMsg = (json::stringNode *) (*settings) ["dataMsg"];
            json::stringNode *posChangedMsg = (json::stringNode *) (*settings) ["posChangedMsg"];
            json::stringNode *cogChangedMsg = (json::stringNode *) (*settings) ["cogChangedMsg"];
            json::stringNode *sogChangedMsg = (json::stringNode *) (*settings) ["sogChangedMsg"];
            json::stringNode *hdgChangedMsg = (json::stringNode *) (*settings) ["hdgChangedMsg"];

            if (pollingInterval != json::nothing) cfg.pollingInterval = (time_t) pollingInterval->getValue ();
            if (timezone != json::nothing) cfg.timezone = (float) timezone->getValue ();
            if (timeout != json::nothing) cfg.timeout = (time_t) timeout->getValue ();
            if (logbookPeriod != json::nothing) cfg.logbookPeriod = (time_t) logbookPeriod->getValue ();
            if (newDataMsg != json::nothing) cfg.newDataMsg = RegisterWindowMessageA (newDataMsg->getValue ());
            if (posChangedMsg != json::nothing) cfg.posChangedMsg = RegisterWindowMessageA (posChangedMsg->getValue ());
            if (cogChangedMsg != json::nothing) cfg.cogChangedMsg = RegisterWindowMessageA (cogChangedMsg->getValue ());
            if (sogChangedMsg != json::nothing) cfg.sogChangedMsg = RegisterWindowMessageA (sogChangedMsg->getValue ());
            if (hdgChangedMsg != json::nothing) cfg.hdgChangedMsg = RegisterWindowMessageA (hdgChangedMsg->getValue ());
        }
        if (sensors != json::nothing) {
            for (auto i = 0; i < sensors->size (); ++ i) {
                json::hashNode *sensor = (json::hashNode *) (*sensors) [i];

                if (sensor != json::nothing) {
                    json::stringNode *type = (json::stringNode *) (*sensor) ["type"];
                    json::stringNode *nic = (json::stringNode *) (*sensor) ["nic"];
                    json::numberNode *port = (json::numberNode *) (*sensor) ["port"];

                    cfg.sensors.emplace_back (type->getValue (), nic->getValue (), port->getValue ());
                }
            }
        }
        if (layout != json::nothing) {
            for (auto i = 0; i < layout->size (); ++ i) {
                json::hashNode *element = (json::hashNode *) (*layout) [i];

                if (element != json::nothing) {
                    json::stringNode *type = (json::stringNode *) (*element) ["type"];
                    json::hashNode *label = (json::hashNode *) (*element) ["label"];
                    json::stringNode *unit = (json::stringNode *) (*element) ["unit"];
                    json::stringNode *orient = (json::stringNode *) (*element) ["orientation"];
                    json::numberNode *id = (json::numberNode *) (*element) ["id"];
                    json::numberNode *x = (json::numberNode *) (*element) ["x"];
                    json::numberNode *y = (json::numberNode *) (*element) ["y"];
                    json::numberNode *width = (json::numberNode *) (*element) ["width"];
                    json::numberNode *height = (json::numberNode *) (*element) ["height"];
                    json::stringNode *image = (json::stringNode *) (*element) ["image"];
                    json::numberNode *offsetX = (json::numberNode *) (*element) ["offsetX"];
                    json::numberNode *offsetY = (json::numberNode *) (*element) ["offsetY"];
                    json::arrayNode *nodes = (json::arrayNode *) (*element) ["nodes"];
                    json::stringNode *color = (json::stringNode *) (*element) ["color"];

                    layoutElementType elemType;
                    if (strcmp (type->getValue (), "tank") == 0) {
                        elemType = layoutElementType::TANK;
                        elemId = (int) id->getValue ();
                    } else if (strcmp (type->getValue (), "meter") == 0) {
                        elemType = layoutElementType::FUELMETER;
                        elemId = (int) id->getValue ();
                    } else if (strcmp (type->getValue (), "image") == 0) {
                        json::stringNode *image = (json::stringNode *) (*element) ["image"];

                        if (strcmp (image->getValue (), "engine") == 0) {
                            elemId = layoutImage::ENGINE;
                        } else {
                            elemId = layoutImage::NONE;
                        }

                        elemType = layoutElementType::IMAGE;
                    } else if (strcmp (type->getValue (), "pipe") == 0) {
                        elemType = layoutElementType::PIPE;
                        elemId = (int) id->getValue ();
                    } else {
                        continue;
                    }

                    layoutOrientation orientation;
                    if (orient == json::nothing) {
                        orientation = layoutOrientation::UNKNOWN; 
                    } else {
                        if (strcmp (orient->getValue (), "horizontal") == 0) {
                            orientation = layoutOrientation::HORIZONTAL;
                        } else if (strcmp (orient->getValue (), "vertical") == 0) {
                            orientation = layoutOrientation::VERTICAL;
                        } else {
                            orientation = layoutOrientation::UNKNOWN; 
                        }
                    }
                    layoutUnit elemUnit;
                    if (strcmp (unit->getValue (), "pixel") == 0)
                        elemUnit = layoutUnit::PIXELS;
                    else if (strcmp (unit->getValue (), "percent") == 0)
                        elemUnit = layoutUnit::PERCENT;
                    else
                        continue;

                    penColor clr;
                    if (color == json::nothing) {
                        clr = penColor::BLACK;
                    } else {
                        if (strcmp (color->getValue (), "black") == 0)
                            clr = penColor::BLACK;
                        else if (strcmp (color->getValue (), "blue") == 0)
                            clr = penColor::BLUE;
                        else if (strcmp (color->getValue (), "red") == 0)
                            clr = penColor::RED;
                        else if (strcmp (color->getValue (), "green") == 0)
                            clr = penColor::GREEN;
                        else if (strcmp (color->getValue (), "blue") == 0)
                            clr = penColor::BLUE;
                        else if (strcmp (color->getValue (), "gray") == 0)
                            clr = penColor::GRAY;
                        else
                            clr = penColor::BLACK;
                    }

                    layoutLabelPos lblPos;
                    std::string lblText;
                    if (label == json::nothing) {
                        lblPos = layoutLabelPos::BELOW;
                    } else {
                        json::stringNode *labelPos = (json::stringNode *) (*label) ["pos"];
                        json::stringNode *labelText = (json::stringNode *) (*label) ["text"];
                        if (labelPos != json::nothing) {
                            if (strcmp (labelPos->getValue (), "above") == 0)
                                lblPos = layoutLabelPos::ABOVE;
                            else if (strcmp (labelPos->getValue (), "below") == 0)
                                lblPos = layoutLabelPos::BELOW;
                            else if (strcmp (labelPos->getValue (), "left") == 0)
                                lblPos = layoutLabelPos::LEFT;
                            else if (strcmp (labelPos->getValue (), "right") == 0)
                                lblPos = layoutLabelPos::RIGHT;
                            else if (labelPos->getValue () [0])
                                continue;
                            else
                                lblPos = layoutLabelPos::BELOW;
                        } else {
                            lblPos = layoutLabelPos::BELOW;
                        }
                        if (labelText != json::nothing) {
                            lblText = labelText->getValue ();
                        }
                    }

                    auto getNumericValue = [] (json::numberNode *node) {
                        return node == json::nothing ? 0 : (int) node->getValue ();
                    };

                    auto newItem = cfg.layout.emplace (
                        elemId,
                        layoutElement (
                            elemType,
                            elemUnit,
                            lblPos,
                            lblText.c_str (),
                            elemId,
                            getNumericValue (x),
                            getNumericValue (y),
                            getNumericValue (width),
                            getNumericValue (height),
                            orientation,
                            getNumericValue (offsetX),
                            getNumericValue (offsetY),
                            clr
                        )
                    );

                    if (nodes != json::nothing) {
                        for (auto i = 0; i < nodes->size (); ++ i) {
                            json::hashNode *nodeItem = (json::hashNode *) nodes->at (i);

                            json::numberNode *nodeX = (json::numberNode *) (*nodeItem) ["x"];
                            json::numberNode *nodeY = (json::numberNode *) (*nodeItem) ["y"];
                            json::numberNode *nodeOffsetX = (json::numberNode *) (*nodeItem) ["offsetX"];
                            json::numberNode *nodeOffsetY = (json::numberNode *) (*nodeItem) ["offsetY"];

                            newItem.first->second.addNode (
                                getNumericValue (nodeX),
                                getNumericValue (nodeY),
                                getNumericValue (nodeOffsetX),
                                getNumericValue (nodeOffsetY)
                            );
                        }
                    }
                }
            }
        }
        if (params != json::nothing) {
            for (auto i = 0; i < params->size (); ++ i) {
                json::hashNode *parameter = (json::hashNode *) (*params) [i];

                if (parameter != json::nothing) {
                    json::numberNode *id = (json::numberNode *) (*parameter) ["id"];
                    json::stringNode *key = (json::stringNode *) (*parameter) ["key"];
                    json::stringNode *name = (json::stringNode *) (*parameter) ["name"];
                    json::numberNode *isNumber = (json::numberNode *) (*parameter) ["isNumber"];
                    json::numberNode *multiplier = (json::numberNode *) (*parameter) ["multiplier"];
                    json::numberNode *group = (json::numberNode *) (*parameter) ["group"];

                    auto& newParam = cfg.params.emplace (
                        (uint8_t) id->getValue (),
                        param (
                            (uint8_t) elemId,
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
        if (paramGroups != json::nothing) {
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
        if (columnMap != json::nothing) {
            for (auto iter = columnMap->begin (); iter != columnMap->end (); ++ iter) {
                json::numberNode *column = (json::numberNode *) iter->second;

                cfg.columnMap.emplace (iter->first, (uint8_t) column->getValue ());
            }
        }
        if (draftAft != json::nothing) cfg.draftAftChannel = draftAft->getValue ();
        if (draftFore != json::nothing) cfg.draftForeChannel = draftFore->getValue ();

        printf ("port: %.f; host: %s\n", port->getValue (), host->getValue ());

        free (buffer);
        fclose (cfgFile);
    }
}
