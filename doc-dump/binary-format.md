- binary format has a header section and a program section, program is split into data and code segments
- the following DEFINES the structure of version 1.0.0

# HEADER
- header Size -  2 bytes
- header Version - 4 bytes (Major - 1 byte, Minor - 1 byte, Revision : 2 bytes )
- Machine Name_Size - 1 byte
- Machine Name - n bytes as per previous field
- Machine Version - 4 bytes as above
- Program Name SIze - 2 bytes
- Program Name - n bytes as a s per field
# PROGRAM
- Data Segment - Size 4 bytes
- Data Segment - n bytes as above
- Code Segment Size - 4 bytes
- Code Segment - n bytes as above


