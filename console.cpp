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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

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

	registerCmd("dump",				WRAP_METHOD(CryoConsole, cmdDump));
	registerCmd("sentences",			WRAP_METHOD(CryoConsole, cmdSentences));
	registerCmd("sound",				WRAP_METHOD(CryoConsole, cmdSound));
	registerCmd("sprite",				WRAP_METHOD(CryoConsole, cmdSprite));
}

CryoConsole::~CryoConsole() {
}

bool CryoConsole::cmdDump(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Decompresses the given HSQ file into a raw uncompressed file\n");
		debugPrintf("  Usage: %s <file name>\n\n", argv[0]);
		debugPrintf("  Example: %s phrase11.hsq\n", argv[0]);
		debugPrintf("  The above will uncompress phrase11.hsq into phrase11.hsq.raw\n");
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	ResourceManager *resMan = _engine->getResourceManager();
	resMan->dumpResource(fileName);

	debugPrintf("%s has been dumped to %s\n", fileName.c_str(), (fileName + ".raw").c_str());
	return true;
}

bool CryoConsole::cmdSentences(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Shows information about a sentence file, or prints a specific sentence from a file\n");
		debugPrintf("  Usage: %s <file name> <sentence number>\n\n", argv[0]);
		debugPrintf("  Example: \"%s phrase12\" - show information on file phrase12.hsq\n", argv[0]);
		debugPrintf("  Example: \"%s phrase12.hsq 0\" - print sentence with index 0 from file phrase12.hsq\n\n", argv[0]);
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Sentences *s = new Sentences(fileName, _engine);
	if (argc == 2) {
		debugPrintf("File contains %d sentences\n", s->count());
	} else {
		if (atoi(argv[2]) >= s->count())
			debugPrintf("Invalid sentence\n");
		else
			debugPrintf("%s\n", s->getSentence(atoi(argv[2]), true).c_str());
	}
	delete s;

	return true;
}

bool CryoConsole::cmdSound(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Plays a sound file (sd*.hsq). Valid sounds are 1-11\n");
		return true;
	}

	uint16 soundId = atoi(argv[1]);
	if (soundId < 1 || soundId > 11) {
		debugPrintf("Invalid sound\n");
		return true;
	}

	char filename[10];
	sprintf(filename, "sd%x.hsq", soundId);

	ResourceManager *resMan = _engine->getResourceManager();
	Common::SeekableReadStream& readS = *resMan->getResource(filename);
	Audio::SoundHandle handle;

	//NEW STYLE
	Audio::RewindableAudioStream *stream = Audio::makeVOCStream(&readS,Audio::FLAG_UNSIGNED);


	_engine->_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &handle, stream, -1, 255);

	return true;
}

bool CryoConsole::cmdSprite(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Shows information about a game sprite (character/background) file\n");
		debugPrintf("  Usage: %s <file name> <frame number> <x> <y>\n\n", argv[0]);
		debugPrintf("  Example: \"%s mirror\" - show information on file mirror.hsq\n", argv[0]);
		debugPrintf("  Example: \"%s mirror.hsq 0\" - display frame number 0 from mirror.hsq at 0, 0\n", argv[0]);
		debugPrintf("  Example: \"%s mirror.hsq 0 100 100\" - display frame number 0 from mirror.hsq at 100, 100\n", argv[0]);
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Sprite *s = new Sprite(fileName, _engine);
	bool showConsole = true;

	uint16 frameCount = s->getFrameCount();
	if (argc == 2) {
		// Show sprite info
		debugPrintf("Frame count: %d\n", frameCount);
		for (int i = 0; i < frameCount; i++) {
			FrameInfo info = s->getFrameInfo(i);
			debugPrintf("%d: offset %d, comp: %d, size: %dx%d, pal offset: %d\n",
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
			debugPrintf("Invalid frame\n");
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
