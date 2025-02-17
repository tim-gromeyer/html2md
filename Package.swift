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
            path: ".",
            sources: [
                "src/html2md.cpp",
                "src/table.h",
            ],
            publicHeadersPath: "include",
            cxxSettings: [
                .unsafeFlags(["-std=c++11"]),
            ]
        ),
    ]
)
