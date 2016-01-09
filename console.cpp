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
 * $URL: file:///svn/p/dunerevival/code/trunk/cryo/console.cpp $
 * $Id: console.cpp 98 2013-12-09 17:11:32Z monsieurouxx $
 *
 */

// Console module

#include "common/system.h"

#include "audio/audiostream.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"

#include "cryo/console.h"
#include "cryo/cryo.h"
#include "cryo/resource.h"
#include "cryo/sentences.h"
#include "cryo/sprite.h"

namespace Cryo {

CryoConsole::CryoConsole(CryoEngine *engine) : GUI::Debugger(),
	_engine(engine) {

	DCmd_Register("dump",				WRAP_METHOD(CryoConsole, cmdDump));
	DCmd_Register("sentences",			WRAP_METHOD(CryoConsole, cmdSentences));
	DCmd_Register("sound",				WRAP_METHOD(CryoConsole, cmdSound));
	DCmd_Register("sprite",				WRAP_METHOD(CryoConsole, cmdSprite));
}

CryoConsole::~CryoConsole() {
}

bool CryoConsole::cmdDump(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Decompresses the given HSQ file into a raw uncompressed file\n");
		DebugPrintf("  Usage: %s <file name>\n\n", argv[0]);
		DebugPrintf("  Example: %s phrase11.hsq\n", argv[0]);
		DebugPrintf("  The above will uncompress phrase11.hsq into phrase11.hsq.raw\n");
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Resource *hsqResource = new Resource(fileName);
	hsqResource->dump(fileName + ".raw");
	delete hsqResource;

	DebugPrintf("%s has been dumped to %s\n", fileName.c_str(), (fileName + ".raw").c_str());
	return true;
}

bool CryoConsole::cmdSentences(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Shows information about a sentence file, or prints a specific sentence from a file\n");
		DebugPrintf("  Usage: %s <file name> <sentence number>\n\n", argv[0]);
		DebugPrintf("  Example: \"%s phrase12\" - show information on file phrase12.hsq\n", argv[0]);
		DebugPrintf("  Example: \"%s phrase12.hsq 0\" - print sentence with index 0 from file phrase12.hsq\n\n", argv[0]);
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Sentences *s = new Sentences(fileName);
	if (argc == 2) {
		DebugPrintf("File contains %d sentences\n", s->count());
	} else {
		if (atoi(argv[2]) >= s->count())
			DebugPrintf("Invalid sentence\n");
		else
			DebugPrintf("%s\n", s->getSentence(atoi(argv[2]), true).c_str());
	}
	delete s;

	return true;
}

bool CryoConsole::cmdSound(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Plays a sound file (sd*.hsq). Valid sounds are 1-11\n");
		return true;
	}

	uint16 soundId = atoi(argv[1]);
	if (soundId < 1 || soundId > 11) {
		DebugPrintf("Invalid sound\n");
		return true;
	}

	char filename[10];
	sprintf(filename, "sd%x.hsq", soundId);

	Resource *res = new Resource(filename);
	Common::SeekableReadStream& readS = *res->_stream;
	Audio::SoundHandle handle;
	/* OLD STYLE 
	int size = res->_stream->size();
	int rate = 0;
	
	byte *data = Audio::loadVOCFromStream(readS, size, rate);
	delete res;

	Audio::RewindableAudioStream *stream = Audio::makeRawStream(data, size, rate, Audio::FLAG_UNSIGNED);
	*/

	//NEW STYLE
	Audio::RewindableAudioStream *stream = Audio::makeVOCStream(&readS,Audio::FLAG_UNSIGNED);


	_engine->_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &handle, stream, -1, 255);

	return true;
}

bool CryoConsole::cmdSprite(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Shows information about a game sprite (character/background) file\n");
		DebugPrintf("  Usage: %s <file name> <frame number> <x> <y>\n\n", argv[0]);
		DebugPrintf("  Example: \"%s mirror\" - show information on file mirror.hsq\n", argv[0]);
		DebugPrintf("  Example: \"%s mirror.hsq 0\" - display frame number 0 from mirror.hsq at 0, 0\n", argv[0]);
		DebugPrintf("  Example: \"%s mirror.hsq 0 100 100\" - display frame number 0 from mirror.hsq at 100, 100\n", argv[0]);
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Sprite *s = new Sprite(fileName, _engine->_system);
	bool showConsole = true;

	uint16 frameCount = s->getFrameCount();
	if (argc == 2) {
		// Show sprite info
		DebugPrintf("Frame count: %d\n", frameCount);
		for (int i = 0; i < frameCount; i++) {
			FrameInfo info = s->getFrameInfo(i);
			DebugPrintf("%d: offset %d, comp: %d, size: %dx%d, pal offset: %d\n",
					i, info.offset, info.isCompressed, info.width, info.height, info.palOffset);
		}
	} else {
		// Draw sprite frame
		//TODO: if any part of the sprite is outside of the screen, the game crashes (graphics.cpp assert)
		_engine->_system->fillScreen(0);
		uint16 frameNumber = atoi(argv[2]);
		uint16 x = (argc > 3) ? atoi(argv[3]) : 0;
		uint16 y = (argc > 4) ? atoi(argv[4]) : 0;

		if (frameNumber >= frameCount) {
			DebugPrintf("Invalid frame\n");
		} else {
			s->setPalette();
			s->drawFrame(frameNumber, x, y);
			showConsole = false;
		}
	}

	delete s;

	return showConsole;
}

} // End of namespace Cryo
