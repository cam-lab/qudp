import qbs 1.0
import qbs.FileInfo

Product {
    name: "qudp"
    type: "staticlibrary"
    files: [ "qudp.cpp",
             "qudp_lib.cpp",
             "qudp.h" ] 


    property string buildVariant: qbs.buildVariant
    property string lib_dir: FileInfo.path(sourceDirectory + "/../../../libs/")

    destinationDirectory: FileInfo.joinPaths(lib_dir, buildVariant)

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["testlib", "network"] }
    cpp.defines: ["ENA_FW_QT"]
    cpp.includePaths: ['../../../include']

    Group {
        name:      "Windows stuff"
        condition: qbs.targetOS.contains("windows")
        cpp.defines: "ENA_WIN_API"
    }
}


