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
 * $Id: resource.cpp
 *
 */

#include "common/memstream.h"
#include "common/system.h"
#include "graphics/palette.h"

#include "cryo/resource.h"
#include "cryo/sprite.h"

namespace Cryo {

Sprite::Sprite(Common::String filename, OSystem *system) : _system(system) {
	_res = new Resource(filename);
}

Sprite::~Sprite() {
	delete _res;
}

void Sprite::setPalette() {
	// The first chunk is the palette chunk
	_res->_stream->seek(0);
	uint16 chunkSize = _res->_stream->readUint16LE();
	uint16 curPos = 2;

	if (chunkSize > _res->_stream->size())
		error("Sprite file is corrupted");

	while (curPos < chunkSize) {
		byte palStart = _res->_stream->readByte();
		byte palCount = _res->_stream->readByte();

		if (palStart == 0xFF && palCount == 0xFF)
			break;

		byte *palChunk = new byte[palCount * 4];	// RGBA
		for (uint i = 0; i < palCount; i++) {
			if (i + palStart > 256)
				break;
			palChunk[i * 4 + 0] = _res->_stream->readByte() << 2;	// R
			palChunk[i * 4 + 1] = _res->_stream->readByte() << 2;	// G
			palChunk[i * 4 + 2] = _res->_stream->readByte() << 2;	// B
			palChunk[i * 4 + 3] = 0;	// A
		}
		_system->getPaletteManager()->setPalette(palChunk, palStart, palCount);
		delete[] palChunk;
	}
}

uint16 Sprite::getFrameCount() {
	// The second chunk is the frame chunk, a list of 16-bit offsets
	_res->_stream->seek(0);

	uint16 chunkSize = _res->_stream->readUint16LE();
	_res->_stream->seek(chunkSize);
	chunkSize = _res->_stream->readUint16LE();

	return (chunkSize - 2) / 2;
}

FrameInfo Sprite::getFrameInfo(uint16 frameIndex) {
	FrameInfo result;
	// First, get the frame count
	// This will place the pointer at the start of the frame table
	uint16 frameCount = getFrameCount();
	uint16 chunkStart = _res->_stream->pos() - 2;
	assert (frameIndex < frameCount);

	// Now, get the offset of the frame
	_res->_stream->skip(frameIndex * 2);
	result.offset = _res->_stream->readUint16LE();
	_res->_stream->seek(chunkStart + result.offset);

	byte b1 = _res->_stream->readByte();
	byte b2 = _res->_stream->readByte();
	result.isCompressed = b2 & 0x80;
	result.width = b1 | ((b2 & 0x7f) << 8);
	result.height = _res->_stream->readByte();
	result.palOffset = _res->_stream->readSByte();

	// width must be divisible by 4
	while (result.width % 4 != 0)
		result.width++;

	return result;
}

void Sprite::drawFrame(uint16 frameIndex, uint16 x, uint16 y) {
	FrameInfo info = getFrameInfo(frameIndex);
	// The pointer is now at the beginning of the frame data
	assert (info.width > 0 && info.height > 0);

	uint32 totalSize = info.width * info.height;

	byte *rect = new byte[totalSize];
	byte *dst = rect;
	uint32 cur = 0;
	byte pixel;
	int count;

	if (!info.isCompressed) {
		byte *buf = new byte[totalSize / 2];
		_res->_stream->read(buf, totalSize / 2);
		while (cur < totalSize) {
			pixel = buf[cur / 2];
			// Data is stored in half bytes
			*dst++ = (pixel & 0xf) + info.palOffset;
			*dst++ = (pixel >> 4) + info.palOffset;
			cur += 2;
			if (cur >= totalSize)
				break;
		}
		delete[] buf;
	} else {
		while (cur < totalSize) {
			// Data is stored in half bytes, with a simple RLE compression
			int8 repetition = _res->_stream->readSByte();
			bool fillSingleValue = (repetition < 0);
			count = ((repetition < 0) ? -repetition : repetition) + 1;
			pixel = fillSingleValue ? _res->_stream->readByte() : 0;

			for (int i = 0; i < count; i++) {
				if (!fillSingleValue)
					pixel = _res->_stream->readByte();

				*dst++ = (pixel & 0xf) + info.palOffset;
				*dst++ = (pixel >> 4) + info.palOffset;
				cur += 2;
				if (cur >= totalSize)
					break;
			}
		}
	}
	
	_system->copyRectToScreen(rect, info.width, x, y, info.width, info.height);

	delete[] rect;
}

} // End of namespace Cryo
