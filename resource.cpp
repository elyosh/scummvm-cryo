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

#include "common/file.h"

#include "cryo/resource.h"

namespace Cryo {

#define HSQ_PACKED_CHECKSUM 171

/**
 * A class that reads both bits and bytes from a stream, with a 16-bit buffer.
 */
class BitByteReader {
public:
	BitByteReader(byte *data) : _data(data), _curBit(0), _queue(0) { }

	byte getBit() {
		if (!_curBit)
			refillQueue();

		byte result = _queue & 0x1;
		_queue >>= 1;
		_curBit--;

		return result;
	}

	byte getByte() {
		return *_data++;
	}

private:
	void refillQueue() {
		_curBit = 16;
		_queue = READ_LE_UINT16(_data);
		_data += 2;
	}

	byte *_data;
	byte _curBit;
	uint16 _queue;
};

Resource::Resource(Common::String filename) {
	Common::File f;
	uint32 origSize = 0;
	byte *origData = 0;

	filename.toUppercase();

	// FIXME: Don't reload the whole file table
	// TODO: Use game flags instead of searching for DUNE.DAT
	// TODO: This isn't really good for large files (e.g. videos),
	// as the file contents are loaded in memory. Consider using
	// SeekableSubReadStream instead
	if (f.open("DUNE.DAT")) {
		// CD version
		uint16 entries = f.readUint16LE();
		char cdFileName[16];
		uint32 cdFileOffset = 0;

		for (uint16 i = 0; i < entries; i++) {
			f.read(cdFileName, 16);
			origSize = f.readUint32LE();
			cdFileOffset = f.readUint32LE();
			f.readByte();	// 0

			if (filename == cdFileName) {
				f.seek(cdFileOffset);
				break;
			}
		}

		if (!cdFileOffset)
			error("File %s not found in dune.dat", filename.c_str());
	} else {
		// Floppy version
		if (f.open(filename)) {
			origSize = f.size();
		} else {
			error("Error reading from file %s", filename.c_str());
		}
	}

	origData = new byte[origSize];
	f.read(origData, origSize);
	f.close();

	_size = origSize;
	_data = origData;

	if (_size < 6)
		error("File %s is too small to be an HSQ file", filename.c_str());

	// Get a checksum of the first 6 bytes
	byte sum = 0;	// sum must be a byte, so that the salt value can overflow it to 0xAB
	for (int i = 0; i < 6; i++)
		sum += _data[i];

	if (sum == HSQ_PACKED_CHECKSUM) {	// HSQ compression
		// Details taken from http://wiki.multimedia.cx/index.php?title=HNM_(1)

		// Read 6 byte header
		_size = READ_LE_UINT16(_data);
		assert (*(_data + 2) == 0);	// must be 0
		uint16 packedSize = READ_LE_UINT16(_data + 3);
		if (packedSize != origSize)
			error("File %s is corrupt - size is %d, it should be %d", filename.c_str(), origSize, packedSize);
		// one byte salt, to adjust checksum to 171 (0xAB)

		_data = new byte[_size];
		memset(_data, 0, _size);

		hsqUnpack(origData + 6, _data);

		// Delete the original packed data
		delete[] origData;
	}	// if (sum == PACKED_CHECKSUM)

	// Create a memory read stream of the final data
	_stream = new Common::MemoryReadStream(_data, _size);
}

Resource::~Resource() {
	delete _stream;
	delete _data;
}

void Resource::hsqUnpack(byte *inData, byte *outData) {
	uint16 count;
	int16 offset;
	BitByteReader br(inData);
	byte *dst = outData;

	while (true) {
		if (br.getBit()) {
			*dst++ = br.getByte();
		} else {
			if (br.getBit()) {
				byte b1 = br.getByte();
				byte b2 = br.getByte();

				count = b1 & 0x7;
				offset = ((b1 >> 3) | (b2 << 5)) - 8192;

				if (!count)
					count = br.getByte();
							
				if (!count)
					break;	// finish the unpacking
			} else {
				count = br.getBit() * 2 ;
				count += br.getBit();
				offset = br.getByte() - 256;
			}

			count += 2;

			byte *src = dst + offset;
			while (count--)
				*dst++ = *src++;
		}
	}
}

void Resource::dump(Common::String outFilename) {
	Common::DumpFile f;
	if (f.open(outFilename)) {
		f.write(_data, _size);
		f.flush();
		f.close();
	} else {
		warning("Error opening %s for output", outFilename.c_str());
	}
}

} // End of namespace Cryo
