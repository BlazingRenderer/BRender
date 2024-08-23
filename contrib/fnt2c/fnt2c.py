#!/usr/bin/env python3

import sys
import re
import itertools
import json
import io
import os
from typing import List, IO


class Font(object):
	def __init__(self, name: str):
		self.name = name
		self.encoding_table = {}
		self.width_table = {}
		self.widths = {}
		self.glyphs = {}



def parse_font(name: str, data) -> Font:
	f = Font(name)

	while True:
		l = next(data)
		m = re.match(rb'd[wd]\s+(\w+)\s+;\s+(\w+)', l)
		if m:
			field = m.group(2).decode('utf-8')
			if field in ['flags', 'glyph_x', 'glyph_y', 'spacing_x', 'spacing_y']:
				f.__setattr__(field, int(m.group(1).decode('utf-8'), 10))
		else:
			break

	return f


def parse_encoding_table(font: Font, data):
	while True:
		l = next(data)
		m = re.match(rb'GLYPH\s+(\w+)\s*;\s+([0-9a-zA-Z]{2})', l)
		if m:
			name = m.group(1).decode('utf-8')
			ordinal = m.group(2).decode('utf-8')
			font.encoding_table[int(ordinal, 16)] = name
		else:
			break


def read_bin_list(out: List[int], s: str):
	for i in s.strip().split(','):
		out.append(int(i, 2))


def parse_width_table(font: Font, data):
	while True:
		l = next(data)
		m = re.match(rb'GWIDTH\s+(\w+)\s*;\s+([0-9a-zA-Z]{2})', l)
		if m:
			name = m.group(1).decode('utf-8')
			ordinal = m.group(2).decode('utf-8')
			font.width_table[int(ordinal, 16)] = name
		else:
			break


def load_font_asm(font_name: str) -> Font:
	font: Font

	with open(font_name, 'rb') as f:
		data = (l.strip() for l in f.readlines())

	while True:
		try:
			l = next(data)
		except StopIteration:
			break

		if l.startswith(b';;'):
			continue

		m = re.match(rb'^__(Font.+)\s+label\s+\w+$', l)
		if m:
			font = parse_font(m.group(1).decode('utf-8'), data)
			continue

		m = re.match(rb'^_Font.+_Encoding\s+label\s+\w+$', l)
		if m:
			parse_encoding_table(font, data)
			continue

		m = re.match(rb'^_Font.+_Width\s+label\s+\w+$', l)
		if m:
			parse_width_table(font, data)
			continue

		m = re.match(rb'^width_(\w+)\s+equ\s+(\d+)h$', l)
		if m:
			name = m.group(1).decode('utf-8')
			width = int(m.group(2).decode('utf-8'), 16)
			font.widths[name] = width
			continue

		m = re.match(rb'^glyph_(\w+)\s+db\s+([01,]+)$', l)
		if m:
			name = m.group(1).decode('utf-8')
			values = []
			read_bin_list(values, m.group(2).decode('utf-8'))
			while True:
				l = next(data)
				m = re.match(rb'^db\s+([01,]+)$', l)
				if m:
					read_bin_list(values, m.group(1).decode('utf-8'))
				else:
					break

			font.glyphs[name] = values
	return font


def write_font_c(font: Font, s: IO):
	print('#include <brender.h>', file=s)
	if font.flags == 1:

		print(file=s)
		print('enum {', file=s)
		for k, v in font.widths.items():
			print(f'    width_{k} = {v},', file=s)
		print('};', file=s)
		print(file=s)

		print(f'const static br_int_8 widths[{len(font.width_table)}] = {{', file=s)
		for k, v in font.width_table.items():
			print(f'    [{k}] = width_{v},', file=s)
		print('};', file=s)
		print(file=s)

	for k, v in font.glyphs.items():
		print(f'const static br_uint_8 glyph_{k}[{len(v)}] = {{', file=s)

		nbytes = (font.widths.get(k, font.glyph_x) // 8) + 1
		if len(v) % nbytes != 0:
			raise Exception('invalid font width/encoding')

		for row in itertools.batched(v, nbytes):
			print(f'    {', '.join(format(i, '#010b') for i in row)},', file=s)

		print('};', file=s)
		print(file=s)

	print(f'const static br_uint_8 *encodings[{len(font.encoding_table)}] = {{', file=s)
	for k, v in font.encoding_table.items():
		print(f'    [{k}] = glyph_{v},', file=s)
	print('};', file=s)
	print(file=s)

	print(f'''br_font _{font.name} = {{
	.flags     = {'BR_FONTF_PROPORTIONAL' if font.flags == 1 else 0},
	.glyph_x   = {font.glyph_x},
	.glyph_y   = {font.glyph_y},
	.spacing_x = {font.spacing_x},
	.spacing_y = {font.spacing_y},
	.width     = {'widths' if font.flags == 1 else 'NULL'},
	.encoding  = encodings,
}};''', file=s)


if __name__ == '__main__':
	if len(sys.argv) != 3:
		print(f'Usage: {sys.argv[0]} <font.asm> <output.c>')
		exit(2)

	input_file = sys.argv[1]
	output_file = sys.argv[2]

	font = load_font_asm(input_file)
	with open(output_file, 'w') as f:
		print(f'''/*
 * Automatically generated from {os.path.basename(input_file)}, do not edit.
 */''', file=f)
		write_font_c(font, f)
