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

#ifndef CRYO_FONT_H
#define CRYO_FONT_H

namespace Cryo {

class Resource;
class Sprite;

class FixedFont {
public:
	FixedFont(Common::String filename, CryoEngine *engine);
	~FixedFont();

	void drawText(Common::String text, uint16 x, uint16 y, byte color);

private:
	Common::SeekableReadStream *_stream;
	// An array of all the character widths. Each character is
	// stored as a bit array, so its width can be 0 - 8 pixels
	byte _charWidth[256];
	CryoEngine *_engine;
};

class SpriteFont {
public:
	SpriteFont(Common::String filename, CryoEngine *engine);
	~SpriteFont();

	void drawText(Common::String text, uint16 x, uint16 y);

private:
	Sprite *_spr;
};

} // End of namespace Cryo
 
#endif
