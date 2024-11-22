/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

namespace edbee {

class TextEditorCommandMap;


/// This class contains the factory edbee command map
class EDBEE_EXPORT FactoryCommandMap {
public:
    void fill(TextEditorCommandMap* cm );
};

} // edbee
