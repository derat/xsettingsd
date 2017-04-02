# -*- python -*-
# Copyright 2009 Daniel Erat <dan@erat.org>
# All rights reserved.

import os
import subprocess

Help('''
Type: 'scons xsettingsd' to build xsettingsd
      'scons dump_xsettings' to build dump_xsettings
      'scons test' to build and run all tests
''')


def run_tests(target, source, env):
  '''Run all test binaries listed in 'source'.'''

  tests = sorted([str(t) for t in source])
  max_length = max([len(t) for t in tests])

  for test in tests:
    padded_name = test + ' ' * (max_length - len(test))

    proc = subprocess.Popen('./%s' % test,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT,
                            close_fds=True)
    [stdout, stderr] = proc.communicate()
    proc.wait()
    if proc.returncode == 0:
      print '%s OK' % padded_name
    else:
      print "%s FAILED\n" % padded_name
      print stdout

run_tests_builder = Builder(action=run_tests)


env = Environment(
    BUILDERS = {
      'RunTests': run_tests_builder,
    })

env.Append(CPPFLAGS=os.environ.get('CPPFLAGS', ''),
           CFLAGS=os.environ.get('CFLAGS', ''),
           CXXFLAGS=os.environ.get('CXXFLAGS', ''),
           LINKFLAGS=os.environ.get('LDFLAGS', ''))

env.Append(CCFLAGS='-Wall -Werror -Wno-narrowing')


srcs = Split('''\
  common.cc
  config_parser.cc
  data_reader.cc
  data_writer.cc
  setting.cc
  settings_manager.cc
''')
libxsettingsd = env.Library('xsettingsd', srcs)
env['LIBS'] = libxsettingsd
env.ParseConfig('pkg-config --cflags --libs x11')

xsettingsd     = env.Program('xsettingsd', 'xsettingsd.cc')
dump_xsettings = env.Program('dump_xsettings', 'dump_xsettings.cc')

Default([xsettingsd, dump_xsettings])


gtest_env = env.Clone()
gtest_env.Append(CCFLAGS='-I/usr/src/gtest')
gtest_env.VariantDir('.', '/usr/src/gtest/src', duplicate=0)
libgtest = gtest_env.Library('gtest', 'gtest-all.cc')

test_env = env.Clone()
test_env.Append(CCFLAGS='-D__TESTING')
test_env['LIBS'] += [libgtest, 'pthread']

tests = []
for file in Glob('*_test.cc', strings=True):
  tests += test_env.Program(file)
test_env.RunTests('test', tests)

