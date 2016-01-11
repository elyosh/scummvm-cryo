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

#include "common/scummsys.h"

#include "cryo/hsq.h"

namespace Cryo {

class BitByteReader {
public:
	BitByteReader(Common::SeekableReadStream *data) : _data(data), _curBit(0), _queue(0) { }

	byte getBit() {
		if (!_curBit)
			refillQueue();

		byte result = _queue & 0x1;
		_queue >>= 1;
		_curBit--;

		return result;
	}

	byte getByte() {
		return _data->readByte();
	}

private:
	void refillQueue() {
		_curBit = 16;
		_queue = _data->readUint16LE();
	}

	Common::SeekableReadStream *_data;
	byte _curBit;
	uint16 _queue;
};


HsqReadStream::HsqReadStream(Common::SeekableReadStream *source)
	: _source(source),
	  // It's convention to set the starting cursor position to blockSize - 16
	  _windowCursor(0x0FEE),
	  _eosFlag(false) {
	// All values up to _windowCursor inits by 0x20
	memset(_window, 0x20, _windowCursor);
	memset(_window + _windowCursor, 0, BLOCK_SIZE - _windowCursor);
}

/* The data is organized in a chunks, 18 and more bytes each.
 * Every chunk starts with the 2-bytes "header" bitmap, followed
 * by 16 or more data bytes.
 * The bits in the bitmap specify how to interpret the respective
 * data bytes of a chunk. If the subsequent bit in the bitmap is
 * set (1), then the corresponding data byte is just copied to an
 * output. If the bitmap bit is zero, then the subsequent bitmap
 * bits and data bytes are used to locate the sequence in a
 * previously extracted data and duplicate it. */

uint32 HsqReadStream::decompressBytes(byte *destination, uint32 numberOfBytes) {
	uint16 count;
	int16 offset;
	BitByteReader br(_source);
	byte *dst = destination;

	while (true) {
		if (br.getBit()) {
			/* 1 - just copy one byte */
			*dst++ = br.getByte();
		} else {
			if (br.getBit()) {
				/* 10 - copy up to 7 bytes of previously extracted
				 * data, from no more than 8192 bytes behind the
				 * current extract position. */
				byte low = br.getByte();
				byte high = br.getByte();

				count = low & 0x7;
				offset = ((low >> 3) | (high << 5)) - 8192;

				if (!count) {
					/* can copy up to 255 bytes */
					count = br.getByte();
				}
							
				if (!count)
					break;	// finish the unpacking
			} else {
				/* 00 - copy up to 3 bytes from the position not
				 * further than 256 bytes behind. */
				count = br.getBit() * 2;
				count += br.getBit();
				offset = br.getByte() - 256;
			}

			count += 2;

			byte *src = dst + offset;
			while (count--)
				*dst++ = *src++;
		}
	}

	return dst - destination;
}

bool HsqReadStream::eos() const {
	return _eosFlag;
}

uint32 HsqReadStream::read(void *dataPtr, uint32 dataSize) {
	uint32 bytesRead = decompressBytes(static_cast<byte *>(dataPtr), dataSize);
	if (bytesRead < dataSize) {
		// Flag that we're at EOS
		_eosFlag = true;
	}

	return dataSize;
}

} // End of namespace Cryo
