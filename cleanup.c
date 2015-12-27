//

void
cleanup() {
    // I want to cleanup before we exit
    // such as resetting the buspirate into a known state (no stuck in the middle of raw SPI mode
    // In SPI1 raw mode send 00, puts us in BBIO1 raw mode
    // in BBIO1 raw mode, send ^O (0x0F) and the BP will reset
    // we get the HiZ> prompt
}
