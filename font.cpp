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

#include "graphics/surface.h"

#include "cryo/resource.h"
#include "cryo/sprite.h"
#include "cryo/font.h"

namespace Cryo {

#define FIXED_FONT_HEIGHT 9
// DOS 437 characters start from ASCII 48 ('0')
// The equivalent ASCII character for '0' in Dune's sprite files is in sprite 15
#define SPRITE_FONT_OFFSET 33
#define SPRITE_FONT_SPACE_WIDTH 16

#define SCREEN_WIDTH 320

FixedFont::FixedFont(Common::String filename, OSystem *system) : _system(system) {
	_res = new Resource(filename);
	_res->_stream->read(_charWidth, 256);
}

FixedFont::~FixedFont() {
	delete _res;
}

void FixedFont::drawText(Common::String text, uint16 x, uint16 y, byte color) {
	uint16 curX = x;
	char curChar;
	byte charLine;
	byte *dest;

	Graphics::Surface *screen = _system->lockScreen();
	byte *scr = (byte *)screen->getPixels();

	for (uint c = 0; c < text.size(); c++) {
		dest = scr + y * SCREEN_WIDTH + curX;
		curChar = text[c];

		_res->_stream->seek(256 + curChar * 9);

		for (uint charY = 0; charY < FIXED_FONT_HEIGHT; charY++) {
			charLine = _res->_stream->readByte();

			for (uint charX = 0; charX < _charWidth[(uint8)curChar]; charX++) {
				if (charLine & 0x80)
					dest[charX] = color;

				charLine <<= 1;
			}

			dest += SCREEN_WIDTH;
		}

		curX += _charWidth[(uint8)curChar];
	}

	_system->unlockScreen();
}

SpriteFont::SpriteFont(Common::String filename, OSystem *system) {
	_spr = new Sprite(filename, system);
}

SpriteFont::~SpriteFont() {
	delete _spr;
}

void SpriteFont::drawText(Common::String text, uint16 x, uint16 y) {
	uint16 curX = x;
	char curChar;

	for (uint i = 0; i < text.size(); i++) {
		curChar = text[i];

		if (curChar == ' ') {
			curX += SPRITE_FONT_SPACE_WIDTH;
		} else {
			_spr->drawFrame(curChar - SPRITE_FONT_OFFSET, curX, y);
			curX += _spr->getFrameInfo(curChar - SPRITE_FONT_OFFSET).width;
		}
	}
}

} // End of namespace Cryo
