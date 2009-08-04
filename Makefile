CC=g++ -g -Wall -Werror

xsettingsd: xsettingsd.cc config_parser.o data_writer.o \
            setting.o settings_manager.o
	$(CC) -o xsettingsd xsettingsd.cc config_parser.o data_writer.o \
	  setting.o settings_manager.o

config_parser.o: config_parser.cc config_parser.h common.h setting.h
	$(CC) -o config_parser.o -c config_parser.cc

data_writer.o: data_writer.cc data_writer.h common.h
	$(CC) -o data_writer.o -c data_writer.cc

setting.o: setting.cc setting.h common.h data_writer.h
	$(CC) -o setting.o -c setting.cc

settings_manager.o: settings_manager.cc settings_manager.h \
                    common.h data_writer.h setting.h
	$(CC) -o settings_manager.o -c settings_manager.cc

config_parser_test: config_parser_test.cc config_parser.h setting.h \
                    config_parser.o data_writer.o setting.o
	$(CC) -D__TESTING -o config_parser_test \
	  config_parser_test.cc config_parser.o data_writer.o setting.o -lgtest

clean:
	rm -f xsettingsd *.o *_test
