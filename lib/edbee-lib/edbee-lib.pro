
TEMPLATE = subdirs

#QMAKE_CXXFLAGS+=-fsanitize=address -fsanitize=bound
#QMAKE_LFLAGS+=-fsanitize=address -fsanitize=bounds



src_lib.subdir = edbee-lib

src_lib_test.subdir = edbee-test
src_lib_test.depends = src_lib


SUBDIRS = \
	src_lib \
	src_lib_test

