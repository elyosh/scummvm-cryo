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

// MIDI and digital music class

#include "cryo/cryo.h"
#include "cryo/resource.h"
#include "cryo/music.h"

#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/substream.h"

namespace Cryo {

#define BUFFER_SIZE 4096
#define MUSIC_SUNSPOT 26

CryoMusicDriver::CryoMusicDriver() : _isGM(false) {
	memset(_channel, 0, sizeof(_channel));
	_masterVolume = 0;
	_nativeMT32 = ConfMan.getBool("native_mt32");

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driver = MidiDriver::createMidi(dev);
	_driverType = MidiDriver::getMusicType(dev);
	if (isMT32())
		_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	this->open();
}

CryoMusicDriver::~CryoMusicDriver() {
	this->close();
	delete _driver;
}

int CryoMusicDriver::open() {
	int retValue = _driver->open();
	if (retValue)
		return retValue;

	if (_nativeMT32)
		_driver->sendMT32Reset();
	else
		_driver->sendGMReset();

	return 0;
}

bool CryoMusicDriver::isOpen(void) const
{
	return (_driver->isOpen());
}


void CryoMusicDriver::setVolume(int volume) {
	volume = CLIP(volume, 0, 255);

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	Common::StackLock lock(_mutex);

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
		}
	}
}

void CryoMusicDriver::send(uint32 b) {
	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_isGM && !isMT32()) {
		// Remap MT32 instruments to General Midi
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	} else if ((b & 0xFFF0) == 0x007BB0) {
		// Only respond to All Notes Off if this channel
		// has currently been allocated
		if (!_channel[channel])
			return;
	}

	if (!_channel[channel])
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
	else
		_channel[channel]->send(b);
}

CryoMusic::CryoMusic(Audio::Mixer *mixer) : _mixer(mixer), _data(0) {
	_currentVolume = 0;
	//OLD STYLE _driver = new MusicDriver();
	_driver = new CryoMusicDriver();

	if (!_driver->isAdlib()) {
	}

	// TODO: Dune has a custom MIDI format (neither SMF nor XMIDI),
	// so a custom parser is needed... thus, die here
	error("TODO: Implement parser for the custom MIDI format");

	_parser = MidiParser::createParser_XMIDI();
	//_parser = MidiParser::createParser_SMF();
	_driver->setGM(false);

	_parser->setMidiDriver(_driver);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
}

CryoMusic::~CryoMusic() {
	_mixer->stopHandle(_musicHandle);
	_driver->setTimerCallback(NULL, NULL);
	delete _driver;
	_parser->setMidiDriver(NULL);
	delete _parser;
	delete _data;
}

void CryoMusic::musicVolumeGaugeCallback(void *refCon) {
	((CryoMusic *)refCon)->musicVolumeGauge();
}


void CryoMusic::onTimer(void *refCon) {
	CryoMusic *music = (CryoMusic *)refCon;
	Common::StackLock lock(music->_driver->_mutex);
	music->_parser->onTimer();
}

void CryoMusic::musicVolumeGauge() {
	int volume;

	_currentVolumePercent += 10;

	if (_currentVolume - _targetVolume > 0) { // Volume decrease
		volume = _targetVolume + (_currentVolume - _targetVolume) * (100 - _currentVolumePercent) / 100;
	} else {
		volume = _currentVolume + (_targetVolume - _currentVolume) * _currentVolumePercent / 100;
	}

	if (volume < 0)
		volume = 1;

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
	_driver->setVolume(volume);

	if (_currentVolumePercent == 100) {
		_currentVolume = _targetVolume;
	}
}

void CryoMusic::setVolume(int volume, int time) {
	_targetVolume = volume;
	_currentVolumePercent = 0;

	if (volume == -1) // Set Full volume
		volume = 255;

	if (time == 1) {
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
		_driver->setVolume(volume);
		_currentVolume = volume;
		return;
	}

}

bool CryoMusic::isPlaying() {
	return _mixer->isSoundHandleActive(_musicHandle) || _parser->isPlaying();
}

void CryoMusic::play(Common::String filename, MusicFlags flags) {
	//TODO: find the new version of function "debug" in the new scummvm
	//debug(2, "Music::play %s, %d", filename.c_str(), flags);

	//if (isPlaying() && _trackNumber == resourceId) {
	//	return;
	//}

	//_trackNumber = resourceId;
	_mixer->stopHandle(_musicHandle);
	_parser->unloadMusic();

	if (flags == MUSIC_DEFAULT)
		flags = MUSIC_NORMAL;

	/*Resource *res = new Resource(filename);

	delete[] _data;
	_data = 0;
	_data = new byte[res->_stream->size()];
	res->_stream->read(_data, res->_stream->size());

	if (!_parser->loadMusic(_data, res->_stream->size()))
		error("Music::play() wrong music resource");

	delete res;

	_parser->setTrack(0);
	_driver->setTimerCallback(this, &onTimer);

	//setVolume(_vm->_musicVolume);

	// Handle music looping
	_parser->property(MidiParser::mpAutoLoop, (flags & MUSIC_LOOP) ? 1 : 0);*/
}

void CryoMusic::pause() {
	_driver->setTimerCallback(NULL, NULL);
}

void CryoMusic::resume() {
	_driver->setTimerCallback(this, &onTimer);
}

void CryoMusic::stop() {
	_driver->setTimerCallback(NULL, NULL);
	_parser->unloadMusic();
	delete[] _data;
	_data = 0;
}

} // End of namespace Cryo

