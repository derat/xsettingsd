# Copyright 2009 Daniel Erat <dan@erat.org>
# All rights reserved.

import os
import subprocess

Help('''
Type: 'scons xsettingsd' to build xsettingsd
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
    proc.wait()
    if proc.returncode == 0:
      print '%s OK' % padded_name
    else:
      print '%s FAILED' % padded_name

run_tests_builder = Builder(action=run_tests)


env = Environment(
    BUILDERS = {
      'RunTests': run_tests_builder,
    },
    ENV=os.environ)
env['CCFLAGS'] = '-Wall -Werror -g'


srcs = Split('''\
  config_parser.cc
  data_writer.cc
  setting.cc
  settings_manager.cc
''')
libxsettingsd = env.Library('xsettingsd', srcs)
env['LIBS'] = libxsettingsd

Default(env.Program('xsettingsd', 'xsettingsd.cc'))


test_env = env.Clone()
test_env.Append(CCFLAGS=' -D__TESTING', LINKFLAGS=' -lgtest')

tests = []
for file in Glob('*_test.cc', strings=True):
  tests += test_env.Program(file)
test_env.RunTests('test', tests)
