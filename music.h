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

// Music class

#ifndef CRYO_MUSIC_H
#define CRYO_MUSIC_H

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/mixer.h"

#include "common/array.h"
#include "common/mutex.h"

namespace Cryo {

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 0x0001,
	MUSIC_DEFAULT = 0xffff
};

class CryoMusicDriver : public MidiDriver {
public:
	CryoMusicDriver();
	~CryoMusicDriver();

	void setVolume(int volume);
	int getVolume() { return _masterVolume; }

	bool isAdlib() { return _driverType == MT_ADLIB; }
	bool isMT32() { return _driverType == MT_MT32 || _nativeMT32; }
	void setGM(bool isGM) { _isGM = isGM; }

	//MidiDriver interface implementation
	int open();
	void close() { _driver->close();}
	void send(uint32 b);
	bool isOpen(void) const;

	void metaEvent(byte type, byte *data, uint16 length) {}

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { _driver->setTimerCallback(timerParam, timerProc); }
	uint32 getBaseTempo()	{ return _driver->getBaseTempo(); }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	Common::Mutex _mutex;

protected:

	static void onTimer(void *data);

	MidiChannel *_channel[16];
	MidiDriver *_driver;
	MusicType _driverType;
	byte _channelVolume[16];
	bool _isGM;
	bool _nativeMT32;

	byte _masterVolume;

	byte *_musicData;
	uint16 *_buf;
	size_t _musicDataSize;
};

class CryoMusic {
public:

	CryoMusic(Audio::Mixer *mixer);
	~CryoMusic();
	bool isPlaying();

	void play(Common::String filename, MusicFlags flags = MUSIC_DEFAULT);
	void pause();
	void resume();
	void stop();

	void setVolume(int volume, int time = 1);
	int getVolume() { return _currentVolume; }

	Common::Array<int32> _songTable;

private:
	Audio::Mixer *_mixer;

	CryoMusicDriver *_driver;
	Audio::SoundHandle _musicHandle;
	uint32 _trackNumber;

	int _targetVolume;
	int _currentVolume;
	int _currentVolumePercent;

	MidiParser *_parser;
	byte *_data;

	static void musicVolumeGaugeCallback(void *refCon);
	static void onTimer(void *refCon);
	void musicVolumeGauge();
	//ByteArray *_currentMusicBuffer;
	//ByteArray _musicBuffer[2];
};

} // End of namespace Saga

#endif
