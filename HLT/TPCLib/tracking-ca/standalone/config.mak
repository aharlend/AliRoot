include						config_common.mak

TARGET						= ca
SUBTARGETS					= libAliHLTTPCCAGPUSA

CPPFILES					= display/opengl.cpp
CXXFILES					= standalone.cxx \
								code/AliHLTTPCCATrack.cxx \
								code/AliHLTTPCCATrackParam.cxx \
								code/AliHLTTPCCATracklet.cxx \
								code/AliHLTTPCCAStartHitsFinder.cxx \
								code/AliHLTTPCCANeighboursCleaner.cxx \
								code/AliHLTTPCCAParam.cxx \
								code/AliHLTTPCCATracker.cxx \
								code/AliHLTTPCCATrackerFramework.cxx \
								code/AliHLTTPCCASliceData.cxx \
								code/AliHLTTPCCASliceOutput.cxx \
								code/AliHLTTPCCAStandaloneFramework.cxx \
								code/AliHLTTPCCATrackletConstructor.cxx \
								code/AliHLTTPCCANeighboursFinder.cxx \
								code/AliHLTTPCCAGrid.cxx \
								code/AliHLTTPCCATrackletSelector.cxx \
								code/AliHLTTPCCAHitArea.cxx \
								code/AliHLTTPCCAMCPoint.cxx \
								code/AliHLTTPCCAMerger.cxx \
								code/AliHLTTPCCAClusterData.cxx \
								code/AliHLTTPCCARow.cxx \
								code/AliHLTTPCCAGPUTracker.cxx \
								merger-ca/AliHLTTPCGMMerger.cxx \
								merger-ca/AliHLTTPCGMSliceTrack.cxx \
								merger-ca/AliHLTTPCGMTrackParam.cxx \
								standalone/AliHLTLogging.cxx \
								standalone/AliHLTTPCTransform.cxx
ASMFILES					= 

CONFIG_OPENGL				= 1

