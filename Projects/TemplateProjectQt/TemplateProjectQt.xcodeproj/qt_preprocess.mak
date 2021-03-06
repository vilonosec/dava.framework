#############################################################################
# Makefile for building: TemplateProjectQt.app/Contents/MacOS/TemplateProjectQt
# Generated by qmake (2.01a) (Qt 4.8.1) on: Mon Jul 16 13:59:03 2012
# Project:  TemplateProjectQt.pro
# Template: app
# Command: ~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/qmake -spec ~/QtSDK/Desktop/Qt/4.8.1/gcc/mkspecs/macx-xcode -o TemplateProjectQt.xcodeproj/project.pbxproj TemplateProjectQt.pro
#############################################################################

MOC       = ~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/moc
UIC       = ~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/uic
LEX       = flex
LEXFLAGS  = 
YACC      = yacc
YACCFLAGS = -d
DEFINES       = -DDAVA_DEBUG -DDDARWIN_NO_CARBON -DFT2_BUILD_LIBRARY -DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB -DQT_SHARED
INCPATH       = -I~/QtSDK/Desktop/Qt/4.8.1/gcc/mkspecs/macx-xcode -I. -I~/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtCore.framework/Versions/4/Headers -I~/QtSDK/Desktop/Qt/4.8.1/gcc/include/QtCore -I~/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtNetwork.framework/Versions/4/Headers -I~/QtSDK/Desktop/Qt/4.8.1/gcc/include/QtNetwork -I~/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtGui.framework/Versions/4/Headers -I~/QtSDK/Desktop/Qt/4.8.1/gcc/include/QtGui -I~/QtSDK/Desktop/Qt/4.8.1/gcc/lib/QtOpenGL.framework/Versions/4/Headers -I~/QtSDK/Desktop/Qt/4.8.1/gcc/include/QtOpenGL -I~/QtSDK/Desktop/Qt/4.8.1/gcc/include -I../dava.framework/Sources/Internal -I../dava.framework/Sources/External -I../dava.framework/Sources/External/Freetype -I../dava.framework/Libs/oggvorbis/include -I../dava.framework/Sources/Libs/include -IClasses -I../dava.framework/Libs/libs -I../dava.framework/Libs/freetype/include -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers -I/System/Library/Frameworks/AGL.framework/Headers -I. -I. -I/usr/local/include -I/System/Library/Frameworks/CarbonCore.framework/Headers -F~/QtSDK/Desktop/Qt/4.8.1/gcc/lib
DEL_FILE  = rm -f
MOVE      = mv -f

IMAGES = 
PARSERS =
preprocess: $(PARSERS) compilers
clean preprocess_clean: parser_clean compiler_clean

parser_clean:
check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compilers: ./moc_mainwindow.cpp ./moc_davaglwidget.cpp ./ui_mainwindow.h ./ui_davaglwidget.h
compiler_objective_c_make_all:
compiler_objective_c_clean:
compiler_moc_header_make_all: moc_mainwindow.cpp moc_davaglwidget.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_mainwindow.cpp moc_davaglwidget.cpp
moc_mainwindow.cpp: mainwindow.h
	~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ mainwindow.h -o moc_mainwindow.cpp

moc_davaglwidget.cpp: Classes/davaglwidget.h
	~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/moc $(DEFINES) $(INCPATH) -D__APPLE__ -D__GNUC__ Classes/davaglwidget.h -o moc_davaglwidget.cpp

compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_rez_source_make_all:
compiler_rez_source_clean:
compiler_uic_make_all: ui_mainwindow.h ui_davaglwidget.h
compiler_uic_clean:
	-$(DEL_FILE) ui_mainwindow.h ui_davaglwidget.h
ui_mainwindow.h: mainwindow.ui
	~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/uic mainwindow.ui -o ui_mainwindow.h

ui_davaglwidget.h: Classes/davaglwidget.ui
	~/QtSDK/Desktop/Qt/4.8.1/gcc/bin/uic Classes/davaglwidget.ui -o ui_davaglwidget.h

compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean compiler_uic_clean 

