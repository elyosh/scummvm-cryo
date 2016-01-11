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

#ifndef CRYO_RESOURCE_H
#define CRYO_RESOURCE_H

#include "common/memstream.h"
#include "cryo/cryo.h"

namespace Cryo {

class Archive;

class DatArchive : public Common::Archive {
public:
	DatArchive(const Common::String &filename);

	virtual bool hasFile(const Common::String &name) const;
	virtual int listMembers(Common::ArchiveMemberList &list) const;
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;

protected:
	Common::SeekableReadStream *_stream;

	struct DatEntry {
		uint32 offset;
		uint32 size;
		char filename[16];
	};

	typedef Common::HashMap<Common::String, DatEntry*, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	FileMap _files;
	Common::String _datFilename;
};

Common::Archive *makeDatArchive(const Common::String &name);

class ResourceManager {
public:
	ResourceManager(bool isCD);
	~ResourceManager();

	Common::SeekableReadStream *getResource(Common::String fileName);
	bool dumpResource(Common::String fileName);

protected:
	void hsqUnpack(Common::SeekableReadStream *inData, byte *outData);

	bool _isCD;
	DatArchive *_archive;
};

} // End of namespace Cryo
 
#endif
