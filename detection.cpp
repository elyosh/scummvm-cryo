/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: 
 * $Id: detection.cpp
 *
 */


#include "engines/advancedDetector.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/util.h"

#include "base/plugins.h"

#include "cryo/cryo.h"

static const PlainGameDescriptor cryoGames[] = {
	{ "dune", "Dune" },
	{ 0, 0 }
};


namespace Cryo {

static const ADGameDescription gameDescriptions[] = {
	// English floppy version
	{
		"dune",
		"",
		{
			{"dunes.hsq", 0, "c290b19cfc87333ed2208fa8ffba655d", 21874},
			{0,0,0,0}
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	// English CD version
	{
		"dune",
		"CD",
		{
			{"dune.dat", 0, "f096565944ab48cf4cb6cbf389384e6f", 397794384},
			{0,0,0,0}
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_CD,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Cryo

/* OLD STYLE
static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Cryo::gameDescriptions,
	// Size of that superset structure
	sizeof(ADGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	cryoGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"cryo",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	GUIO0(),
	// Maximum directory depth
	1,
	// List of directory globs
	0
};
*/

class CryoMetaEngine : public AdvancedMetaEngine {
public:
	//OLD STYLE CryoMetaEngine() : AdvancedMetaEngine(detectionParams) {}
	//NEW STYLE
	CryoMetaEngine() : AdvancedMetaEngine(Cryo::gameDescriptions, sizeof(ADGameDescription), cryoGames) {}

	virtual const char *getName() const {
		return "Cryo Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Cryo Engine (C) Cryo Interactive Entertainment";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool CryoMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		false;
}

bool Cryo::CryoEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool CryoMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Cryo::CryoEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(CRYO)
	REGISTER_PLUGIN_DYNAMIC(CRYO, PLUGIN_TYPE_ENGINE, CryoMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CRYO, PLUGIN_TYPE_ENGINE, CryoMetaEngine);
#endif
