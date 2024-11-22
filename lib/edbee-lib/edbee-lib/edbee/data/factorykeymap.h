/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

namespace edbee {

class TextEditorKeyMap;

/**
 * This class can fill the texteditor keymap with the factory defaults
 */
class EDBEE_EXPORT FactoryKeyMap {
public:
    void fill(TextEditorKeyMap* km );
};


} // edbee
