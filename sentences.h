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

#ifndef CRYO_SENTENCES_H
#define CRYO_SENTENCES_H

namespace Cryo {

class Sentences {
public:
	Sentences(Common::String filename, CryoEngine *engine);
	~Sentences();

	uint16 count() const { return _sentenceCount; }
	Common::String getSentence(uint16 index, bool printableOnly = false);

private:
	Common::SeekableReadStream *_stream;
	uint16 _sentenceCount;
	CryoEngine *_engine;
};

} // End of namespace Cryo
 
#endif
