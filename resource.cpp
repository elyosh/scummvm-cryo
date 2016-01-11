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

#include "common/file.h"
#include "common/debug.h"
#include "common/substream.h"
#include "common/archive.h"

#include "cryo/resource.h"
#include "cryo/hsq.h"

namespace Cryo {

#define HSQ_PACKED_CHECKSUM 171

DatArchive::DatArchive(const Common::String &filename) : _datFilename(filename) {
	Common::File datFile;

	if (!datFile.open(_datFilename)) {
		warning("DatArchive::DatArchive(): Could not find archive file %s", _datFilename.c_str());
		return;
	}

	uint16 entries = datFile.readUint16LE();

	DatEntry entry;
	DatEntry *entr;

	for (uint16 i = 0; i < entries; i++) {
		datFile.read(&entry.filename, 16);

		entry.size = datFile.readUint32LE();
		entry.offset = datFile.readUint32LE();

		entr = new DatEntry(entry);

		_files[entry.filename] = entr;

		datFile.readByte();
		//if (_fileTable[i].offset != 0)
		//	debug("Entry %d: name: %s size: %d offset: %d", i, _fileTable[i].fileName, _fileTable[i].size, _fileTable[i].offset);
	}
}

bool DatArchive::hasFile(const Common::String &name) const {
	return _files.contains(name);
}

int DatArchive::listMembers(Common::ArchiveMemberList &list) const {
	int matches = 0;

	FileMap::const_iterator it = _files.begin();
	for ( ; it != _files.end(); ++it) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(it->_value->filename, this)));
		matches++;
	}

	return matches;
}

const Common::ArchiveMemberPtr DatArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *DatArchive::createReadStreamForMember(const Common::String &name) const {
	if (!_files.contains(name)) {
		return 0;
	}

	DatEntry *entry = _files[name];

	Common::File *archive = new Common::File();
	if (!archive->open(_datFilename)) {
		delete archive;
		return NULL;
	}

	return new Common::SeekableSubReadStream(archive, entry->offset, entry->offset + entry->size, DisposeAfterUse::YES);
}

Common::Archive *makeDatArchive(const Common::String &name) {
	return new DatArchive(name);
}


ResourceManager::ResourceManager(bool isCD) : _isCD(isCD) {
	if (_isCD) {
		_archive = (DatArchive *)makeDatArchive("DUNE.DAT");
	} else {
		_archive = 0;
	}
}

ResourceManager::~ResourceManager() {
	delete _archive;
}

Common::SeekableReadStream *ResourceManager::getResource(Common::String fileName) {
	Common::SeekableReadStream *rsrc = NULL;
	Common::SeekableReadStream *res = NULL;

	if (_isCD) {
		rsrc = _archive->createReadStreamForMember(fileName);
	} else {
		Common::File* file = new Common::File();
		file->open(fileName);
		rsrc = file;
	}

	if (!rsrc)
		error("Could not get file %s", fileName.c_str());

	byte sum = 0;	// sum must be a byte, so that the salt value can overflow it to 0xAB
	for (int i = 0; i < 6; i++)
		sum += rsrc->readByte();

	rsrc->seek(0);

	if (sum == HSQ_PACKED_CHECKSUM) {
		uint16 unpackedSize = rsrc->readUint16LE();
		assert (rsrc->readByte() == 0);
		uint16 packedSize = rsrc->readUint16LE();
		rsrc->readByte(); // Salt byte for checksum

		if (packedSize != rsrc->size())
			error("File %s is corrupt - size is %d, it should be %d", fileName.c_str(), rsrc->size(), packedSize);

		byte *unpackData = new byte[unpackedSize];

		HsqReadStream hsqStream(rsrc);
		uint32 unpacked = hsqStream.read(unpackData, unpackedSize);
		res = new Common::MemoryReadStream(unpackData, unpacked, DisposeAfterUse::YES);
	} else {
		rsrc->seek(0);
		res = rsrc;
	}

	return res;
}

bool ResourceManager::dumpResource(Common::String fileName) {
	Common::SeekableReadStream *rsrc = getResource(fileName);
	uint16 size = rsrc->size();

	byte *data = new byte[size];

	rsrc->read(data, size);

	debug("Dumping %s size %d", fileName.c_str(), size);

	Common::DumpFile f;
	if (f.open(fileName + ".raw")) {
		f.write(data, size);
		f.flush();
		f.close();
	}
	delete[] data;

	return true;
}

} // End of namespace Cryo
