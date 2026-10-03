/*
 *   GtkHash - Deepin File Manager integration
 *
 *   This file is part of GtkHash.
 *
 *   GtkHash is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   GtkHash is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GtkHash. If not, see <https://gnu.org/licenses/gpl-2.0.txt>.
 */

#ifndef GTKHASH_DFM_INTEGRATION_H
#define GTKHASH_DFM_INTEGRATION_H

#include <stdbool.h>

// Check whether the Deepin File Manager is present on this system
bool dfm_integration_is_supported(void);

// Install (enabled) or remove (disabled) the Deepin File Manager
// context-menu integration files. Returns true on success.
bool dfm_integration_apply(bool enabled);

#endif
