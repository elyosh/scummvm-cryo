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
 * $Id: cryo.cpp
 *
 */

#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "gui/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
 
#include "engines/util.h"

#include "cryo/console.h"
#include "cryo/cryo.h"
#include "cryo/font.h"
#include "cryo/resource.h"
#include "cryo/sentences.h"
#include "cryo/sprite.h"

namespace Cryo {
 
CryoEngine::CryoEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().
 
	// Do not initialize graphics here
 
	// However this is the place to specify all default directories
	//const Common::FSNode gameDataDir(ConfMan.get("path"));
	//SearchMan.addSubDirectoryMatching(gameDataDir, "sound");
 
	// Here is the right place to set up the engine specific debug levels
	//DebugMan.addDebugChannel(kQuuxDebugExample, "example", "this is just an example for a engine specific debug level");
	//DebugMan.addDebugChannel(kQuuxDebugExample2, "example2", "also an example");
 
	// Don't forget to register your random source
	//OLDSTYLE 
	//g_eventRec.registerRandomSource(_rnd, "cryo");
	//NEWSTYLE
	_rnd = new Common::RandomSource("cryo_randomseed");
	//debug("CryoEngine::CryoEngine");
}
 
CryoEngine::~CryoEngine() {
	// Dispose your resources here
	//debug("CryoEngine::~CryoEngine");
 
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}
 
Common::Error CryoEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);
 
	// Create debugger console. It requires GFX to be initialized
	_console = new CryoConsole(this);
 
	// Additional setup.
	//debug("CryoEngine::init\n");
 
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	// Show something
	Sprite *s = new Sprite("intds.hsq", _system);
	s->setPalette();
	s->drawFrame(0, 0, 92);
	delete s;

	SpriteFont *sf = new SpriteFont("generic.hsq", _system);
	sf->drawText("DUNE TEST", 100, 50);
	delete sf;

	Common::String charFile = isCD() ? "dnchar.bin" : "dunechar.hsq";
	FixedFont *f = new FixedFont(charFile, _system);
	f->drawText("DUNE TEST", 100, 120, 10);
	delete f;

	// Update the screen so that its contents can be shown
	_system->updateScreen();

	// Your main even loop should be (invoked from) here.
	//debug("CryoEngine::go: Hello, World!\n");
 	while (!shouldQuit()) {
		// Open the debugger window, if requested
		while (eventMan->pollEvent(event)) {
			if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_d) {
				_console->attach();
				_console->onFrame();
			}
		}

		// TODO: Do something...
		
		_system->delayMillis(10);
	}

	// This test will show up if -d1 and --debugflags=example are specified on the commandline
	//debugC(1, kQuuxDebugExample, "Example debug call");
 
	// This test will show up if --debugflags=example or --debugflags=example2 or both of them and -d3 are specified on the commandline
	//debugC(3, kQuuxDebugExample | kQuuxDebugExample2, "Example debug call two");
 
	return Common::kNoError;
}

bool CryoEngine::isCD() { 
	return _gameDescription->flags & ADGF_CD;
}

} // End of namespace Cryo
