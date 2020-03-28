/*
  setup.h - Library for setting up xbee network and 328 base pcb
  interrupt pin, baudrate, software serial pins
*/
#ifndef setup_h
#define setup_h

// address of destination
#define ADDR_B1  0x00
#define ADDR_B2  0x13
#define ADDR_B3  0xA2
#define ADDR_B4  0x00
#define ADDR_B5  0x41
#define ADDR_B6  0x4E
#define ADDR_B7  0x65
#define ADDR_B8  0x93


// all addresses  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF // broadcast
// xbee1 address  0x00, 0x13, 0xA2, 0x00, 0x41, 0x25, 0xA4, 0x79 // sleep coordinator
// xbee2 address  0x00, 0x13, 0xA2, 0x00, 0x41, 0x25, 0xA4, 0x81
// xbee3 address  0x00, 0x13, 0xA2, 0x00, 0x41, 0x4E, 0x65, 0x93
// xbee4 address  0x00, 0x13, 0xA2, 0x00, 0x41, 0x4E, 0x65, 0x8D
// xbee5 address  0x00, 0x13, 0xA2, 0x00, 0x41, 0x4E, 0x65, 0x8E
// xbee6 address  0x00, 0x13, 0xA2, 0x00, 0x41, 0x25, 0xA4, 0x95

#endif
