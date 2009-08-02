xsettingsd: xsettingsd.cc settings_manager.h data_writer.o \
            setting.o settings_manager.o
	g++ -o xsettingsd xsettingsd.cc data_writer.o setting.o \
	  settings_manager.o

data_writer.o: data_writer.cc data_writer.h common.h
	g++ -o data_writer.o -c data_writer.cc

setting.o: setting.cc setting.h common.h data_writer.h
	g++ -o setting.o -c setting.cc

settings_manager.o: settings_manager.cc settings_manager.h \
                    common.h data_writer.h setting.h
	g++ -o settings_manager.o -c settings_manager.cc

clean:
	rm -f xsettingsd *.o
