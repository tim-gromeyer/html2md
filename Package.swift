// swift-tools-version:5.5
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "html2md",
    products: [
        .library(name: "html2md", targets: ["html2md"]),
    ],
    targets: [
        .target(
            name: "html2md",
            dependencies: ["html2md_cpp"],
            path: ".",
            sources: [
                "objc/html2md_objc.mm",
            ],
            publicHeadersPath: "objc/include",
            cxxSettings: [
                // header is inherited from html2md_cpp
                // we should compile this objc file with c++11
                .unsafeFlags(["-std=c++11"]),
            ]
        ),
        .target(
            name: "html2md_cpp",
            path: ".",
            sources: [
                "src/html2md.cpp",
                "src/table.cpp",
            ],
            publicHeadersPath: "include",
            cxxSettings: [
                .unsafeFlags(["-std=c++11"]),
                .unsafeFlags(["-Wno-parentheses", "-Wno-conversion"]),
            ]
        ),
    ]
)
