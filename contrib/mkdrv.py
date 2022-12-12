#!/usr/bin/env python

import os
import sys
import subprocess
import tempfile
import copy
import re
import argparse
from typing import List

def get_short_path(long_name):
	#print '%-16s%s' % ('long_name: ', long_name)
	for_cmd = 'for %I in ("' + long_name + '") do echo %~sI'
	#print '%-16s%s' % ('for_cmd: ', for_cmd)

	p = os.popen(for_cmd)
	short_name = p.readlines()[-1] # last line from for command

	if p.close():
		print('Error calling shell command "for"')
		raise Exception
		
	return short_name.replace('\n', '').replace('\r', '')


def remove_paths(includes: List[str]) -> List[str]:
	newpaths = []

	for path in includes:
		comps = path.split(os.sep)

		#print(comps)

		if 'UnitTest' in comps:
			#print('skip')
			continue
		elif 'cppwinrt' in comps:
			#print('skip')
			continue

		newpaths.append(path)
	return newpaths


def dedup_paths(paths: List[str]) -> List[str]:
	return sorted(set(paths), key=lambda x: paths.index(x))


def run_h2inc(filepath: str, includes: List[str]) -> str:
	h2inc = os.path.join(os.path.dirname(__file__), 'H2INC.EXE')

	with tempfile.TemporaryDirectory() as tmpdir:
		procpath = os.path.join(tmpdir, "_h2inc.tmp")

		env_includes = remove_paths([p for p in os.environ.get('INCLUDE', '').split(';') if p != ''])

		arg_includes = [get_short_path(path) for path in includes]
		#env_includes = [get_short_path(path) for path in [p for p in os.environ.get('INCLUDE', '').split(';') if p != '']]
		env_includes = [get_short_path(path) for path in env_includes]

		env_includes = dedup_paths(env_includes)
		arg_includes = dedup_paths(arg_includes)

		params = [
			h2inc, '-nologo', '-G3', '-Zp4', '-w', '-c', '-WIN32',
			'-DBASED_FLOAT=1', '-D_WIN32', '-D__VISUALC', '-D__H2INC__', '-D_NO_PROTOTYPES',
			#'-DWIN32_LEAN_AND_MEAN'
		]

		params += [f'-I{path}' for path in arg_includes]

		params += [f'-Fa{procpath}', os.path.basename(filepath)]

		newenv = copy.deepcopy(os.environ)
		newenv.pop('INCLUDE', None)
		#if len(env_includes) > 0:
		#	newenv['INCLUDE'] = ";".join(env_includes)

		#print(f'Executing h2inc: {params}')
		#print('  INCLUDE: ', env_includes)

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

#print(args)
pass1 = run_h2inc(args.infile, args.include_directories)
pass2 = fix_h2inc(pass1)

with open(args.outfile, 'w') as f:
	f.write(pass2)
