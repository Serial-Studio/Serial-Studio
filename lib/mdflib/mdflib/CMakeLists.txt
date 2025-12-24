# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ${BUILD_SHARED_LIBS})

add_library(mdf
        src/mdfblock.cpp src/mdfblock.h
        src/mdffile.cpp ../include/mdf/mdffile.h
        src/idblock.cpp src/idblock.h
        src/hd3block.cpp src/hd3block.h
        src/tx3block.cpp src/tx3block.h
        src/pr3block.cpp src/pr3block.h
        src/tr3block.cpp src/tr3block.h
        src/sr3block.cpp src/sr3block.h
        src/dg3block.cpp src/dg3block.h
        src/cg3block.cpp src/cg3block.h
        src/cn3block.cpp src/cn3block.h
        src/cc3block.cpp src/cc3block.h
        src/cd3block.cpp src/cd3block.h
        src/ce3block.cpp src/ce3block.h
        src/mdfreader.cpp ../include/mdf/mdfreader.h
        src/mdf3file.cpp src/mdf3file.h
        src/hd4block.cpp src/hd4block.h
        src/tx4block.cpp src/tx4block.h
        src/md4block.cpp src/md4block.h
        src/fh4block.cpp src/fh4block.h
        src/ch4block.cpp src/ch4block.h
        src/at4block.cpp src/at4block.h
        src/ev4block.cpp src/ev4block.h
        src/dg4block.cpp src/dg4block.h
        src/cg4block.cpp src/cg4block.h
        src/si4block.cpp src/si4block.h
        src/cn4block.cpp src/cn4block.h
        src/cc4block.cpp src/cc4block.h
        src/ca4block.cpp src/ca4block.h
        src/dt4block.cpp src/dt4block.h
        src/sr4block.cpp src/sr4block.h
        src/rd4block.cpp src/rd4block.h
        src/sd4block.cpp src/sd4block.h
        src/dl4block.cpp src/dl4block.h
        src/dz4block.cpp src/dz4block.h
        src/hl4block.cpp src/hl4block.h
        src/mdf4file.cpp src/mdf4file.h
        src/datablock.cpp src/datablock.h
        src/datalistblock.cpp src/datalistblock.h
        src/blockproperty.cpp src/blockproperty.h
        src/mdf4timestamp.cpp src/mdf4timestamp.h
        src/iattachment.cpp ../include/mdf/iattachment.h
        ../include/mdf/ichannel.h src/ichannel.cpp
        ../include/mdf/idatagroup.h src/idatagroup.cpp
        ../include/mdf/ichannelgroup.h src/ichannelgroup.cpp
        src/channelobserver.h src/channelobserver.cpp
        ../include/mdf/ichannelobserver.h src/ichannelobserver.cpp
        ../include/mdf/ichannelconversion.h src/ichannelconversion.cpp
        src/mdf3writer.h src/mdf3writer.cpp
        src/dt3block.cpp src/dt3block.h
        src/iheader.cpp ../include/mdf/iheader.h
        ../include/mdf/samplerecord.h
        src/mdf4writer.cpp src/mdf4writer.h
        src/mdfwriter.cpp ../include/mdf/mdfwriter.h
        src/mdffactory.cpp ../include/mdf/mdffactory.h
        src/ifilehistory.cpp ../include/mdf/ifilehistory.h
        src/imetadata.cpp ../include/mdf/imetadata.h
        src/etag.cpp ../include/mdf/etag.h
        src/ievent.cpp ../include/mdf/ievent.h
        src/ichannelhierarchy.cpp ../include/mdf/ichannelhierarchy.h
        src/isourceinformation.cpp ../include/mdf/isourceinformation.h
        src/ichannelarray.cpp ../include/mdf/ichannelarray.h
        src/mdfhelper.cpp ../include/mdf/mdfhelper.h
        src/dv4block.cpp src/dv4block.h
        src/di4block.cpp src/di4block.h
        src/rv4block.cpp src/rv4block.h
        src/ri4block.cpp src/ri4block.h
        src/ld4block.cpp src/ld4block.h
        src/cryptoutil.cpp ../include/mdf/cryptoutil.h
        src/zlibutil.cpp ../include/mdf/zlibutil.h
        src/ixmlfile.cpp src/ixmlfile.h
        src/ixmlnode.cpp src/ixmlnode.h
        src/writexml.cpp src/writexml.h
        src/xmlnode.cpp src/xmlnode.h
        src/expatxml.cpp src/expatxml.h
        src/mdflogstream.cpp ../include/mdf/mdflogstream.h
        src/littlebuffer.cpp src/littlebuffer.h
        src/bigbuffer.cpp src/bigbuffer.h
        src/platform.cpp src/platform.h
        src/iblock.cpp ../include/mdf/iblock.h
        src/vlsddata.cpp src/vlsddata.h
        src/mdfbuslogger.cpp src/mdfbuslogger.h
        src/canmessage.cpp ../include/mdf/canmessage.h
        src/dbchelper.cpp src/dbchelper.h
        src/isampleobserver.cpp  ../include/mdf/isampleobserver.h
        src/isamplereduction.cpp ../include/mdf/isamplereduction.h
        src/readcache.cpp src/readcache.h
        src/dgrange.cpp src/dgrange.h
        src/cgrange.cpp src/cgrange.h
        src/mdfconverter.cpp src/mdfconverter.h
        src/timestamp.cpp  ../include/mdf/imdftimestamp.h
        src/mdf3timestamp.cpp src/mdf3timestamp.h
        src/linmessage.cpp ../include/mdf/linmessage.h
        src/linconfigadapter.cpp ../include/mdf/linconfigadapter.h
        src/ethmessage.cpp ../include/mdf/ethmessage.h
        src/ethconfigadapter.cpp ../include/mdf/ethconfigadapter.h
        src/writecache.cpp src/writecache.h
        src/samplequeue.cpp src/samplequeue.h
        src/writer4samplequeue.cpp src/writer4samplequeue.h
        src/convertersamplequeue.cpp src/convertersamplequeue.h
        src/mdcomment.cpp ../include/mdf/mdcomment.h
        src/mdstandardattribute.cpp ../include/mdf/mdstandardattribute.h
        src/mdproperty.cpp ../include/mdf/mdproperty.h
        src/mdlist.cpp ../include/mdf/mdlist.h
        src/mdenumerate.cpp ../include/mdf/mdenumerate.h
        src/mdstring.cpp ../include/mdf/mdstring.h
        src/mdalternativename.cpp ../include/mdf/mdalternativename.h
        src/mdformula.cpp ../include/mdf/mdformula.h
        src/mdsyntax.cpp ../include/mdf/mdsyntax.h
        src/mdvariable.cpp ../include/mdf/mdvariable.h
        src/mdextension.cpp ../include/mdf/mdextension.h
        src/hdcomment.cpp ../include/mdf/hdcomment.h
        src/hounitspecification.cpp ../include/mdf/hounitspecification.h
        src/hoadmindata.cpp ../include/mdf/hoadmindata.h
        src/horevision.cpp ../include/mdf/horevision.h
        src/honamedetails.cpp ../include/mdf/honamedetails.h
        src/hophysicaldimension.cpp ../include/mdf/hophysicaldimension.h
        src/hounitgroup.cpp ../include/mdf/hounitgroup.h
        src/hounit.cpp ../include/mdf/hounit.h
        src/hointerval.cpp ../include/mdf/hointerval.h
        src/fhcomment.cpp ../include/mdf/fhcomment.h
        src/chcomment.cpp ../include/mdf/chcomment.h
        src/atcomment.cpp ../include/mdf/atcomment.h
        src/mdnumber.cpp ../include/mdf/mdnumber.h
        src/evcomment.cpp ../include/mdf/evcomment.h
        src/dgcomment.cpp ../include/mdf/dgcomment.h
        src/cgcomment.cpp ../include/mdf/cgcomment.h
        src/sicomment.cpp ../include/mdf/sicomment.h
        src/cncomment.cpp ../include/mdf/cncomment.h
        src/hocompumethod.cpp ../include/mdf/hocompumethod.h
        src/hoenumerates.cpp ../include/mdf/hoenumerates.h
        src/hoscaleconstraint.cpp ../include/mdf/hoscaleconstraint.h
        src/hocompuscale.cpp ../include/mdf/hocompuscale.h
        src/cccomment.cpp ../include/mdf/cccomment.h
        src/ccunit.cpp
        ../include/mdf/ccunit.h
        src/cnunit.cpp
        ../include/mdf/cnunit.h
        src/iconfigadapter.cpp
        ../include/mdf/iconfigadapter.h
        src/canconfigadapter.cpp
        ../include/mdf/canconfigadapter.h
        ../include/mdf/mdfenumerates.h
        src/mdfenumerates.cpp
        src/mostmessage.cpp
        ../include/mdf/mostmessage.h
        src/mostconfigadapter.cpp
        ../include/mdf/mostconfigadapter.h
        src/flexraymessage.cpp
        ../include/mdf/flexraymessage.h
        src/flexrayconfigadapter.cpp
        ../include/mdf/flexrayconfigadapter.h
        src/canbusobserver.cpp
        ../include/mdf/canbusobserver.h
)

set(CMAKE_POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

if(VCPKG)
  find_package(ZLIB REQUIRED)
  target_link_libraries(mdf PRIVATE ZLIB::ZLIB)

  find_package(expat CONFIG REQUIRED)
  target_link_libraries(mdf PRIVATE expat::expat)
else()
  include("../script/zlib.cmake")
  include("../script/expat.cmake")
  
  target_include_directories(mdf PRIVATE ${ZLIB_INCLUDE_DIRS})
  target_include_directories(mdf PRIVATE ${EXPAT_INCLUDE_DIRS})

  target_compile_definitions(mdf PRIVATE XML_STATIC)
  target_link_libraries(mdf ${ZLIB_LIBRARIES} ${EXPAT_LIBRARIES})
endif()

target_include_directories(
  mdf PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(mdf PROPERTIES COMPILE_FLAGS "-fPIC")
elseif(MSVC)
  target_compile_definitions(mdf PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS)
endif()

set(MDF_PUBLIC_HEADERS
    ../include/mdf/atcomment.h
    ../include/mdf/canconfigadapter.h
    ../include/mdf/canmessage.h
    ../include/mdf/cccomment.h
    ../include/mdf/ccunit.h
    ../include/mdf/cgcomment.h
    ../include/mdf/chcomment.h
    ../include/mdf/cncomment.h
    ../include/mdf/cnunit.h
    ../include/mdf/cryptoutil.h
    ../include/mdf/dgcomment.h
    ../include/mdf/etag.h
    ../include/mdf/ethconfigadapter.h
    ../include/mdf/ethmessage.h
    ../include/mdf/evcomment.h
    ../include/mdf/fhcomment.h
    ../include/mdf/flexrayconfigadapter.h
    ../include/mdf/flexraymessage.h
    ../include/mdf/hdcomment.h
    ../include/mdf/hoadmindata.h
    ../include/mdf/hocompumethod.h
    ../include/mdf/hocompuscale.h
    ../include/mdf/hoenumerates.h
    ../include/mdf/hointerval.h
    ../include/mdf/honamedetails.h
    ../include/mdf/hophysicaldimension.h
    ../include/mdf/horevision.h
    ../include/mdf/hoscaleconstraint.h
    ../include/mdf/hounit.h
    ../include/mdf/hounitgroup.h
    ../include/mdf/hounitspecification.h
    ../include/mdf/iattachment.h
    ../include/mdf/iblock.h
    ../include/mdf/ichannel.h
    ../include/mdf/ichannelarray.h
    ../include/mdf/ichannelconversion.h
    ../include/mdf/ichannelgroup.h
    ../include/mdf/ichannelhierarchy.h
    ../include/mdf/ichannelobserver.h
    ../include/mdf/iconfigadapter.h
    ../include/mdf/idatagroup.h
    ../include/mdf/ievent.h
    ../include/mdf/ifilehistory.h
    ../include/mdf/iheader.h
    ../include/mdf/imdftimestamp.h
    ../include/mdf/imetadata.h
    ../include/mdf/isampleobserver.h
    ../include/mdf/isourceinformation.h
    ../include/mdf/itimestamp.h
    ../include/mdf/linconfigadapter.h
    ../include/mdf/linmessage.h
    ../include/mdf/mdalternativename.h
    ../include/mdf/mdcomment.h
    ../include/mdf/mdenumerate.h
    ../include/mdf/mdextension.h
    ../include/mdf/mdfenumerates.h
    ../include/mdf/mdffactory.h
    ../include/mdf/mdffile.h
    ../include/mdf/mdfhelper.h
    ../include/mdf/mdflogstream.h
    ../include/mdf/mdformula.h
    ../include/mdf/mdfreader.h
    ../include/mdf/mdfwriter.h
    ../include/mdf/mdlist.h
    ../include/mdf/mdnumber.h
    ../include/mdf/mdproperty.h
    ../include/mdf/mdstandardattribute.h
    ../include/mdf/mdstring.h
    ../include/mdf/mdsyntax.h
    ../include/mdf/mdvariable.h
    ../include/mdf/mostconfigadapter.h
    ../include/mdf/mostmessage.h
    ../include/mdf/isamplereduction.h
    ../include/mdf/samplerecord.h
    ../include/mdf/sicomment.h
    ../include/mdf/zlibutil.h)
set_target_properties(mdf PROPERTIES PUBLIC_HEADER "${MDF_PUBLIC_HEADERS}")
get_target_property(PH1 mdf PUBLIC_HEADER)
message(STATUS "MDF Target Includes: " "${PH1}")

# Only install when mdflib is the top-level project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  if(WIN32)
    install(
      TARGETS mdf
      EXPORT MdfLibTargets
      LIBRARY DESTINATION mdf/lib
      ARCHIVE DESTINATION mdf/lib
      RUNTIME DESTINATION mdf/bin
      PUBLIC_HEADER DESTINATION mdf/include/mdf)
  else()
    install(
      TARGETS mdf
      EXPORT MdfLibTargets
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      RUNTIME DESTINATION bin
      PUBLIC_HEADER DESTINATION include/mdf)
  endif()
endif()
