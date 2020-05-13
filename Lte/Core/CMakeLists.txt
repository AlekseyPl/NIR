cmake_minimum_required(VERSION 3.10)
project(Lte_Core)

set(SOURCES
	Source/Correlator/LteCorrelator.cpp
	Source/Correlator/FilterAfr.cpp
	Source/Correlator/PssCorrelator.cpp
	Source/Correlator/SssCorrelator.cpp
	Source/Correlator/SssCorrelatorSlow.cpp
	Source/Correlator/SyncCode.cpp
	Source/Correlator/SyncFilter.cpp
	Source/Decoder/Crc.cpp
	Source/Decoder/Decoder.cpp
	Source/Decoder/Deinterleaver.cpp
	Source/Decoder/RateRecovery.cpp
	Source/Decoder/SoftConvDecoder.cpp
	Source/Decoder/SoftTurboDecoder.cpp
	Source/Decoder/TurboCodeSupport.cpp
	Source/LogicalProc/ControlChansMap.cpp
	Source/LogicalProc/DciSupport.cpp
	Source/LogicalProc/PbchProcessing.cpp
	Source/LogicalProc/PbchIndex.cpp
	Source/LogicalProc/PdcchProcessing.cpp
	Source/LogicalProc/PcfichIndex.cpp
	Source/LogicalProc/PcfichProcessing.cpp
	Source/LogicalProc/PdschProcessing.cpp
	Source/LogicalProc/Riv.cpp
	Source/LogicalProc/TranspBlock.cpp
	Source/LogicalProc/VrbMap.cpp
	Source/Ofdm/Equalizer.cpp
	Source/Ofdm/Estimator.cpp
	Source/Ofdm/LteOfdmDemodulator.cpp
	Source/Support/LteSupport.cpp
	Source/Support/LteTypes.cpp
	Source/Support/PnSequence.cpp
	Source/Support/RsShift.cpp
	Source/LteStream.cpp
	)

set(HEADERS
	Include/Lte/Core/Correlator/LteCorrelator.h
	Include/Lte/Core/Correlator/FilterAfr.h
	Include/Lte/Core/Correlator/PssCorrelator.h
	Include/Lte/Core/Correlator/SssCorrelator.h
	Include/Lte/Core/Correlator/SssCorrelatorSlow.h
	Include/Lte/Core/Correlator/SyncCode.h
	Include/Lte/Core/Correlator/SyncFilter.h
	Include/Lte/Core/Decoder/Crc.h
	Include/Lte/Core/Decoder/Decoder.h
	Include/Lte/Core/Decoder/Deinterleaver.h
	Include/Lte/Core/Decoder/RateRecovery.h
	Include/Lte/Core/Decoder/SoftConvDecoder.h
	Include/Lte/Core/Decoder/SoftTurboDecoder.h
	Include/Lte/Core/Decoder/TurboCodeSupport.h
	Include/Lte/Core/LogicalProc/ControlChansMap.h
	Include/Lte/Core/LogicalProc/DciSupport.h
	Include/Lte/Core/LogicalProc/PbchIndex.h
	Include/Lte/Core/LogicalProc/PbchProcessing.h
	Include/Lte/Core/LogicalProc/PbchSymbStore.h
	Include/Lte/Core/LogicalProc/PdcchProcessing.h
	Include/Lte/Core/LogicalProc/PcfichIndex.h
	Include/Lte/Core/LogicalProc/PcfichProcessing.h
	Include/Lte/Core/LogicalProc/PdschProcessing.h
	Include/Lte/Core/LogicalProc/Riv.h
	Include/Lte/Core/LogicalProc/TranspBlock.h
	Include/Lte/Core/LogicalProc/VrbMap.h
	Include/Lte/Core/Ofdm/Equalizer.h
	Include/Lte/Core/Ofdm/Estimator.h
	Include/Lte/Core/Ofdm/LteOfdmDemodulator.h
	Include/Lte/Core/Support/CellInfo.h
	Include/Lte/Core/Support/LteConst.h
	Include/Lte/Core/Support/LteDemConst.h
	Include/Lte/Core/Support/LteSupport.h
	Include/Lte/Core/Support/LteTypes.h
	Include/Lte/Core/Support/PnSequence.h
	Include/Lte/Core/Support/Quadruplet.h
	Include/Lte/Core/Support/RsShift.h
	Include/Lte/Core/Support/TrBlockSizeTable.h
	Include/Lte/Core/LteStream.h)
if(CONFIG_LTE_ABONS)
	set(SOURCES ${SOURCES}
		Source/Uplink/Correlation.cpp
		Source/Uplink/CorrelationV2.cpp
		Source/Uplink/MarkerCrc.cpp
		Source/Uplink/MarkerDecoder.cpp
		Source/Uplink/MarkerDeinterleaver.cpp
		Source/Uplink/VbsDetector.cpp
		Source/Uplink/VbsMarker.cpp
		Source/Uplink/SibAbonParser.cpp
		)

	set(HEADERS ${HEADERS}
		Include/Lte/Core/Uplink/Correlation.h
		Include/Lte/Core/Uplink/CorrelationV2.h
		Include/Lte/Core/Uplink/MarkerConst.h
		Include/Lte/Core/Uplink/MarkerCrc.h
		Include/Lte/Core/Uplink/MarkerDecoder.h
		Include/Lte/Core/Uplink/MarkerDeinterleaver.h
		Include/Lte/Core/Uplink/VbsDetector.h
		Include/Lte/Core/Uplink/VbsMarker.h
		Include/Lte/Core/Uplink/SibAbonParser.h
		)
endif()
set(PUBLIC_TARGET_DEPENDECIES ${PUBLIC_TARGET_DEPENDECIES} math system)


if (NOT "${HEADERS}${SOURCES}" STREQUAL "")
	set(Lte_Core_not_empty ON PARENT_SCOPE)

	add_library(Lte_Core STATIC ${SOURCES} ${HEADERS})

	target_include_directories(Lte_Core
		PRIVATE Include/Lte/Core/
		PUBLIC Include)

	target_link_libraries(Lte_Core
	PRIVATE ${PRIVATE_TARGET_DEPENDECIES}
	PUBLIC ${PUBLIC_TARGET_DEPENDECIES})
endif()
