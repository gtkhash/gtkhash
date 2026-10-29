/*
 *   Copyright (C) 2007-2026 Tristan Heaven <tristan@tristanheaven.net>
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

#ifndef GTKHASH_EXTENSIONS_NAUTILUS_PROPERTIES_HASH_H
#define GTKHASH_EXTENSIONS_NAUTILUS_PROPERTIES_HASH_H

#include "properties.h"

void gtkhash_properties_hash_start(struct model_s *model);
void gtkhash_properties_hash_stop(struct model_s *model);
void gtkhash_properties_hash_init(struct model_s *model);
void gtkhash_properties_hash_deinit(struct model_s *model);

#endif
