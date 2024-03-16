 // EventProvider.mc 
 // This is the header section.
 // The following are the categories of events.
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_RUNTIME                 0x2
#define FACILITY_STUBS                   0x3
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: NETWORK_CATEGORY
//
// MessageText:
//
// Network Event
//
#define NETWORK_CATEGORY                 ((WORD)0x00000001L)

//
// MessageId: RADIUS_CATEGORY
//
// MessageText:
//
// Radius Event
//
#define RADIUS_CATEGORY                  ((WORD)0x00000002L)

//
// MessageId: PACKET_GENERATION_CATEGORY
//
// MessageText:
//
// Packet Generation Event
//
#define PACKET_GENERATION_CATEGORY       ((WORD)0x00000003L)

//
// MessageId: FREE_RADIUS_TEST_CATEGORY
//
// MessageText:
//
// Packet Generation Event
//
#define FREE_RADIUS_TEST_CATEGORY        ((WORD)0x00000004L)

//
// MessageId: FREE_RADIUS_GENERAL_CATEGORY
//
// MessageText:
//
// Packet Generation Event
//
#define FREE_RADIUS_GENERAL_CATEGORY     ((WORD)0x00000005L)

 // The following are the message definitions.
//
// MessageId: RC_AVPAIR_GEN
//
// MessageText:
//
// rc_avpair_gen: %1
//
#define RC_AVPAIR_GEN                    ((DWORD)0x00020100L)

//
// MessageId: RADIUS_MESSAGE
//
// MessageText:
//
// radius: %1
//
#define RADIUS_MESSAGE                   ((DWORD)0x00020101L)

//
// MessageId: NETWORK_MESSAGE
//
// MessageText:
//
// rc_avpair_gen: %1
//
#define NETWORK_MESSAGE                  ((DWORD)0x00020102L)

//
// MessageId: FREE_RADIUS_MESSAGE
//
// MessageText:
//
// freeradius: %1
//
#define FREE_RADIUS_MESSAGE              ((DWORD)0x00020103L)

