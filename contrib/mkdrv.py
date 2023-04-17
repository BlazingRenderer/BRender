#!/usr/bin/env python

import os
import sys
import subprocess
import tempfile
import copy
import re
import argparse
import shutil
from typing import List


def build_local_include_dir(tmpdir: str, paths: List[str]) -> str:
	incpath = os.path.join(tmpdir, 'include')

	os.makedirs(incpath, exist_ok=True)
	for p in paths:
		for path, dirs, files in os.walk(p):
			for f in files:
				_, ext = os.path.splitext(f)
				if ext.lower() not in ['.h', '.cgh']:
					continue

				file = os.path.join(path, f)
				shutil.copyfile(file, os.path.join(incpath, f))

	return incpath


def run_h2inc(filepath: str, includes: List[str]) -> str:
	h2inc = os.path.join(os.path.dirname(__file__), 'H2INC.EXE')

	with tempfile.TemporaryDirectory() as tmpdir:
		procpath = os.path.join(tmpdir, "_h2inc.tmp")

		incpath = build_local_include_dir(tmpdir, includes)

		params = [
			h2inc, '-nologo', '-G3', '-Zp4', '-w', '-c', '-WIN32',
			'-DBASED_FLOAT=1', '-D_WIN32',
			'-D__VISUALC', '-D__H2INC__', '-D_NO_PROTOTYPES',
			#'-DWIN32_LEAN_AND_MEAN',
			f'-I{incpath}',
			f'-Fa{procpath}', os.path.basename(filepath)
		]

		newenv = copy.deepcopy(os.environ)
		newenv.pop('INCLUDE', None)

		print(f'Executing h2inc: {params}', file=sys.stderr)

		res = subprocess.run(args=params, env=newenv, cwd=os.path.dirname(filepath))
		if res.returncode != 0:
			exit(res.returncode)

		with open(procpath, 'r') as f:
			return f.read()


def fix_h2inc(data: str) -> str:
	data = re.sub(r'(br_fvector[234](_f|_x)?\s*STRUCT)\s*4t', r'\1 ; 4t H2INCFIX', data)
	data = re.sub(r'(fmt_vertex[234](_f|_x)?\s*STRUCT)\s*4t', r'\1 ; 4t H2INCFIX', data)
	data = re.sub(r'(v11face[234](_f|_x)?\s*STRUCT)\s*4t', r'\1 ; 4t H2INCFIX', data)
	data = re.sub(r'(v11group[234](_f|_x)?\s*STRUCT)\s*4t', r'\1 ; 4t H2INCFIX', data)
	return data


ap = argparse.ArgumentParser(sys.argv[0], description='H2INC wrapper', prefix_chars='/')
ap.add_argument('/I', dest='include_directories', action='append', type=str, default=[])
ap.add_argument('infile')
ap.add_argument('outfile')

args = ap.parse_args()

print(args, file=sys.stderr)
pass1 = run_h2inc(args.infile, args.include_directories)
pass2 = fix_h2inc(pass1)

with open(args.outfile, 'w') as f:
	f.write(pass2)
