cmake_minimum_required(VERSION 3.10)
project(Lte_Core)

set(SOURCES
#		./Common/Crc.cpp
#		./Common/DciSupport.cpp
#		./Common/Deinterleaver.cpp
#		./Common/PbchIndex.cpp
#		./Common/PcfichIndex.cpp
#		./Common/PnSequence.cpp
#		./Common/RateRecovery.cpp
#		./Common/RsShift.cpp
#		./Common/TranspBlock.cpp
#		./Common/TurboCodeSupport.cpp
#	    ./Common/SssCorrelator.cpp
		./Common/SssCorrelatorSlow.cpp
		./Common/LteSupport.cpp
		./Common/LteTypes.cpp
		./Common/PssCorrelator.cpp
		./Common/SyncCode.cpp
		./Common/SyncFilter.cpp
		./Common/FilterAfr.cpp
		./LteOfdmDemodulator.cpp
		./LteCorrelator.cpp
#		./ControlChansMap.cpp
#		./Decoder.cpp
#		./Equalizer.cpp
#		./Estimator.cpp
#		./LteStream.cpp
#		./PbchProcessing.cpp
#		./PcfichProcessing.cpp
#		./PdcchProcessing.cpp
#		./PdschProcessing.cpp
#		./Riv.cpp
#		./SoftConvDecoder.cpp
#		./SoftTurboDecoder.cpp
#		./VrbMap.cpp
		)

	set(HEADERS
		./Common/CellInfo.h
#		./Common/Crc.h
#		./Common/DciSupport.h
#		./Common/Deinterleaver.h
		./Common/LteConst.h
		./Common/LteDemConst.h
		./Common/LteSupport.h
		./Common/LteTypes.h
#		./Common/PbchIndex.h
#		./Common/PbchSymbStore.h
#		./Common/PcfichIndex.h
#		./Common/PnSequence.h
#		./Common/Quadruplet.h
#		./Common/RateRecovery.h
#		./Common/RsShift.h
#		./Common/TrBlockSizeTable.h
#		./Common/TranspBlock.h
#		./Common/TurboCodeSupport.h
		./Common/SssCorrelatorSlow.h
#	    ./Common/SssCorrelator.h
		./Common/PssCorrelator.h
		./Common/SyncCode.h
		./Common/SyncFilter.h
		./Common/FilterAfr.h
		./LteCorrelator.h
		./LteOfdmDemodulator.h
#		./ControlChansMap.h
#		./Decoder.h
#		./Equalizer.h
#		./Estimator.h
#		./LteStream.h
#		./PbchProcessing.h
#		./PcfichProcessing.h
#		./PdcchProcessing.h
#		./PdschProcessing.h
#		./Riv.h
#		./SoftConvDecoder.h
#		./SoftTurboDecoder.h
#		./VrbMap.h
		)

add_library(Lte_Core STATIC ${SOURCES} ${HEADERS})

target_include_directories(Lte_Core
	PUBLIC ./ ../ ${LIBTMS_ROOT})

target_link_libraries(Lte_Core PUBLIC math system)

