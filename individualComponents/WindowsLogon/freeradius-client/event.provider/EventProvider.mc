; // EventProvider.mc 

; // This is the header section.


SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )


FacilityNames=(System=0x0:FACILITY_SYSTEM
               Runtime=0x2:FACILITY_RUNTIME
               Stubs=0x3:FACILITY_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
              )

LanguageNames=(English=0x409:MSG00409)


; // The following are the categories of events.

MessageIdTypedef=WORD

MessageId=0x1
SymbolicName=NETWORK_CATEGORY
Language=English
Network Event
.

MessageId=0x2
SymbolicName=RADIUS_CATEGORY
Language=English
Radius Event
.

MessageId=0x3
SymbolicName=PACKET_GENERATION_CATEGORY
Language=English
Packet Generation Event
.

MessageId=0x4
SymbolicName=FREE_RADIUS_TEST_CATEGORY
Language=English
Packet Generation Event
.

MessageId=0x5
SymbolicName=FREE_RADIUS_GENERAL_CATEGORY
Language=English
Packet Generation Event
.

; // The following are the message definitions.

MessageIdTypedef=DWORD

MessageId=0x100
Facility=Runtime
SymbolicName=RC_AVPAIR_GEN
Language=English
rc_avpair_gen: %1
.

MessageId=0x101
Facility=Runtime
SymbolicName=RADIUS_MESSAGE
Language=English
radius: %1
.

MessageId=0x102
Facility=Runtime
SymbolicName=NETWORK_MESSAGE
Language=English
rc_avpair_gen: %1
.

MessageId=0x103
Facility=Runtime
SymbolicName=FREE_RADIUS_MESSAGE
Language=English
freeradius: %1
.
