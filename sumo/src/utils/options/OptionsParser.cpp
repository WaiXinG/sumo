/****************************************************************************/
/// @file    OptionsParser.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 17 Dec 2001
/// @version $Id$
///
// Parses the command line arguments
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.dlr.de/
// Copyright (C) 2001-2015 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <iostream>
#include <cstring>
#include "Option.h"
#include "OptionsCont.h"
#include "OptionsParser.h"
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
bool
OptionsParser::parse(int argc, char** argv) {
    bool ok = true;
    if (argc == 2 && argv[1][0] != '-') {
        // special case only one parameter is handled like config
        check("-c", argv[1], ok);
        return ok;
    }
    for (int i = 1; i < argc;) {
        try {
            int add;
            // try to set the current option
            if (i < argc - 1) {
                add = check(argv[i], argv[i + 1], ok);
            } else {
                add = check(argv[i], 0, ok);
            }
            i += add;
        } catch (ProcessError& e) {
            WRITE_ERROR("On processing option '" + std::string(argv[i]) + "':\n " + e.what());
            i++;
            ok = false;
        }
    }
    return ok;
}


int
OptionsParser::check(const char* arg1, const char* arg2, bool& ok) {
    // the first argument should be an option
    // (only the second may be a free string)
    if (!checkParameter(arg1)) {
        ok = false;
        return 1;
    }

    OptionsCont& oc = OptionsCont::getOptions();
    // process not abbreviated switches
    if (!isAbbreviation(arg1)) {
        std::string tmp(arg1 + 2);
        size_t idx1 = tmp.find('=');
        // check whether a parameter was submitted
        if (idx1 != std::string::npos) {
            ok &= oc.set(tmp.substr(0, idx1), tmp.substr(idx1 + 1));
        } else {
            if (arg2 == 0 || (oc.isBool(convert(arg1 + 2)) && arg2[0] == '-')) {
                ok &= oc.set(convert(arg1 + 2), "true");
            } else {
                ok &= oc.set(convert(arg1 + 2), convert(arg2));
                return 2;
            }
        }
        return 1;
    }
    // go through the abbreviated switches
    for (int i = 1; arg1[i] != 0; i++) {
        // set boolean switches
        if (oc.isBool(convert(arg1[i]))) {
            if (arg2 == 0 || arg2[0] == '-' || arg1[i + 1] != 0) {
                ok &= oc.set(convert(arg1[i]), "true");
            } else {
                ok &= oc.set(convert(arg1[i]), convert(arg2));
                return 2;
            }
            // set non-boolean switches
        } else {
            // check whether the parameter comes directly after the switch
            //  and process if so
            if (arg2 == 0 || arg1[i + 1] != 0) {
                ok &= processNonBooleanSingleSwitch(oc, arg1 + i);
                return 1;
                // process parameter following after a space
            } else {
                ok &= oc.set(convert(arg1[i]), convert(arg2));
                // option name and attribute were in two arguments
                return 2;
            }
        }
    }
    // all switches within the current argument were boolean switches
    return 1;
}


bool
OptionsParser::processNonBooleanSingleSwitch(OptionsCont& oc, const char* arg) {
    if (arg[1] == '=') {
        if (strlen(arg) < 3) {
            WRITE_ERROR("Missing value for parameter '" + std::string(arg).substr(0, 1) + "'.");
            return false;
        } else {
            return oc.set(convert(arg[0]), std::string(arg + 2));
        }
    } else {
        if (strlen(arg) < 2) {
            WRITE_ERROR("Missing value for parameter '" + std::string(arg) + "'.");
            return false;
        } else {
            return oc.set(convert(arg[0]), std::string(arg + 1));
        }
    }
}


bool
OptionsParser::checkParameter(const char* arg1) {
    if (arg1[0] != '-') {
        WRITE_ERROR("The parameter '" + std::string(arg1) + "' is not allowed in this context.\n Switch or parameter name expected.");
        return false;
    }
    return true;
}


bool
OptionsParser::isAbbreviation(const char* arg1) {
    return arg1[1] != '-';
}


std::string
OptionsParser::convert(const char* arg) {
    std::string s(arg);
    return s;
}


std::string
OptionsParser::convert(char abbr) {
    char buf[2];
    buf[0] = abbr;
    buf[1] = 0;
    std::string s(buf);
    return buf;
}



/****************************************************************************/

