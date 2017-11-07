import qbs 1.0

Project {
    name: "qudp" 

    references: [
        "src/qudp-lib.qbs",
        "test/test_rx/test_rx.qbs",
        "test/test_tx/test_tx.qbs"
    ]
}

