import os

env = Environment()
env.Append(
    ENV = {'PKG_CONFIG_PATH' : os.environ['PKG_CONFIG_PATH']},
    CFLAGS = ['-ggdb3', '-Wall', '-Werror', '-O0'])

env.Default('bin/sc-exp')

Export(['env'])

SConscript('src/SConscript')
