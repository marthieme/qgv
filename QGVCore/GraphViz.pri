#GraphViz librairie
DEFINES += WITH_CGRAPH
INCLUDEPATH += private
QMAKE_CXXFLAGS += -DQGVCORE_LIB

unix {
 CONFIG += link_pkgconfig
 PKGCONFIG += libcdt libgvc libcgraph libgraph
}
win32 {
 #Configure Windows GraphViz path here :
 CONFIG(release, debug|release){
    GRAPHVIZ_PATH = "C:/Program Files/Graphviz_2.41.20170524.x64_release"
 }else{
    GRAPHVIZ_PATH = "C:/Program Files/Graphviz_2.41.20170524.x64_debug"
 }
 DEFINES += WIN32_DLL
 DEFINES += GVDLL
 INCLUDEPATH += $$GRAPHVIZ_PATH/include/graphviz


# INCLUDEPATH += D:\Workspace\Qt\github_forks\GraphViz\lib\cgraph
# INCLUDEPATH += D:\Workspace\Qt\github_forks\GraphViz\lib\cdt
# INCLUDEPATH += D:\Workspace\Qt\github_forks\GraphViz\lib\gvc
# INCLUDEPATH += D:\Workspace\Qt\github_forks\GraphViz\lib\common
# INCLUDEPATH += D:\Workspace\Qt\github_forks\GraphViz\lib\pathplan

 LIBS += -L$$GRAPHVIZ_PATH/lib -lcgraph -lgvc -lcdt

# win32
# LIBS += -L$$GRAPHVIZ_PATH/lib/release/lib -lgvc -lcgraph -lcdt
}
