import os, glob

env = Environment(ENV={'PATH': os.environ['PATH']})

env.Replace(CXX='clang++')
env.Append(CPPPATH = ['/opt/local/include/', 'libdrunkard/include'])
env.Append(CCFLAGS='-Wall -Wextra -std=c++11 -g -fPIC')
env.Append(LINKFLAGS='-Wl,-rpath,.')
env.Append(LIBPATH=['.', 'libdrunkard/lib'])

env.Program('bilebio', glob.glob('*.cpp'), LIBS=['drunkard', 'sfml-graphics', 'sfml-system', 'sfml-window'])
#['jc', 'allegro_main', 'allegro', 'allegro_font', 'allegro_image'])
