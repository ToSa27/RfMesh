//***************************************************************************//
//
// COPYRIGHT NOTICE (zlib license)
//
// Loosely based on the example application provided by HopeRF
//
// (c) Wouter van Ooijen - wouter@voti.nl
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//***************************************************************************//

#ifndef _RFM70_H_
#define _RFM70_H_

//***************************************************************************//
//
//! \defgroup lowlevel low level interface
//!
//! The low-level interface provides read and write access to the 
//! commands and registers of the RFM70.
//!
//! When a register ( < 0x20 ) is specified for a read or write command
//! the appropriate read or write command bits are added automatically.
//!
//! To use any of these methods, (except init() itself)
//! the interface and the module must have been initialized by
//! an init() call.
//!
//! Besides the registers shown here (bank 0) the rfm70 also has a
//! set of registers in bank 1. These bank 1 registers are initialized by
//! the init() call. If you want to do this yourself: the datasheet
//! shows the required values, but in a very confusing way. The HopeRF 
//! example code is a better reference. No (or very scarce) explanation 
//! is given for these values.
//!
//! For most users, especially novices, it is recommended to use the
//! high level interaface instead of this low level interface.
//
//***************************************************************************//

//***************************************************************************//
//
//! \defgroup highlevel high level interface
//!
//! The high-level interface provides methods for using the rfm70 module.
//! These methods are implemneted by calling the appropriate low level
//! methods. 
//! When possible, it is recommended to use only these high level methods.
//! But when a functionality is needed that is missing it can be implemented
//! using the low level interface.
//!
//! To use any of these methods, (except init() itself)
//! the interface and the module must have been initialized by
//! an init() call.
//
//***************************************************************************//

#include "rfm70reg.h"

//! version of this rfm70 library
//
//! \ingroup highlevel
#define RFM70_LIB_VERSION "V1.02 (2011-12-31)"

//! type of rfm70 (transmit or receive) buffer
//
//! \ingroup highlevel
typedef unsigned char rfm70_buffer [ RFM70_MAX_PACKET_LEN ];

#include "pins.h"	
   
//! interface to an RFM70 module
class rfm70 {

private:
   pins::output_pin &sclk;      
   pins::output_pin &mosi;      
   pins::input_pin  &miso;      
   pins::output_pin &csn;      
   pins::output_pin &ce;     
    
    // void(*wait_ms)(unsigned int);
    // void(*wait_us)(unsigned int);
   
   unsigned char SPI_RW( unsigned char value );    

public:
   //! constructor: create and RFM70 object
   //
   //! \ingroup highlevel
   //! This constructor creates an RFM70 object from the 
   //! pins that connect to the RFM70 and a delay function.
   //! This constructor calls init().
   rfm70( 
      pins::output_pin &sclk,
      pins::output_pin &mosi,
      pins::input_pin  &miso,
      pins::output_pin &csn,
      pins::output_pin &ce

      //pins::uc_pin &sclk,
      //pins::uc_pin &mosi,
      //pins::uc_pin  &miso,
      //pins::uc_pin &csn,
      //pins::uc_pin &ce
      
      // ,void(*wait_ms)(unsigned int)
      // ,void(*wait_us)(unsigned int)
      
   ):      
      sclk( sclk ),
      mosi( mosi ),
      miso( miso ),
      csn(  csn ),
      ce(   ce )
      // ,wait_ms( wait_ms )
      // ,wait_us( wait_us )
   {}     

private:   
   void bank( unsigned char b );
   void init_bank1();
   
public:   
   

//! initialize the library and the rfm70 module
//
//! \ingroup lowlevel
//! This method must be called before any other rfm70 
//! method is called. It can also be called later (maybe even
//! periodically) to re-initialize the interafce and the module.
//!
//! The rfm70 is initialized to
//! - 2 byte CRC
//! - power up
//! - receive mode
//! - auto-acknowledge on all pipes enabled
//! - pipes 0 and 1 are enabled, others disabled
//! - use 5 byte addresses
//! - auto retransmission delay 4000 ms, retry 15 times
//! - use channel 10
//! - air data rate 1Mbit power 5dbm, LNA gain high
//! - use some fixed address
void init( void );

//! read a single-byte command or register
//
//! \ingroup lowlevel
//! This method reads and returns the a single-byte (8 bit) 
//! RFM70 command or register reg. 
unsigned char register_read( unsigned char reg );

//! read a multi-byte command or register
//
//! \ingroup lowlevel
//! This method reads length bytes (8 bit each) from the RFM70 
//! command or register reg into the buffer buf. 
void buffer_read(
   unsigned char reg,
   unsigned char *buf,
   unsigned char length
);

//! write a single-byte command or register
//
//! \ingroup lowlevel
//! This method writes the single-byte (8 bit) val to
//! the  RFM70 command or register reg. 
void register_write( unsigned char reg, unsigned char val );

//! write a multi-byte command or register
//
//! \ingroup lowlevel
//! This method writes length bytes (8 bit each) from 
//! the buffer buf into the RFM70 
//! command or register reg. 
void buffer_write(
   char reg,
   const unsigned char *buf,
   unsigned char length
);

//***************************************************************************//
//
// high-level interface
//
//***************************************************************************//

//! report whether the rfm70 module is present
//
//! \ingroup highlevel
//! This method tests whether the rfm70 module is present.
//! It does so by reading the status register, and verifying that
//! an activate 0x53 command toggles the RBANK bit
//! in the status register.
bool is_present( void );

//! switch the rfm70 to transmit mode
//
//! \ingroup highlevel
//! This method flushes the transmit queue,
//! and switches the rfm70 to transmit mode.
void mode_transmit( void );

//! switch the rfm70 to receive mode
//
//! \ingroup highlevel
//! This method flushes the receive queue,
//! and switches the rfm70 to transmit mode.
void mode_receive( void );

//! switch the rfm70 to standby mode
//
//! \ingroup highlevel
//! This method puts the rfm70 in standby I mode,
//! which reduces the power consumption 
//! (50 uA max).
//! mode_powerdown() reduces the power consumption
//! even further, but requires a longer (but unspecified?)
//! time to return to active mode.
void mode_standby( void );

//! switch the rfm70 to power down mode
//
//! \ingroup highlevel
//! This method puts the rfm70 in power down mode,
//! which reduces the power consumption to a minimum
//! ( 3 uA max). 
void mode_powerdown( void );

//! set the rfm70 lna gain to low
//
//! \ingroup highlevel
//! This sets the LNA gain of the receiver to the low
//! level (-20 dB compared to the 'high' level).
void lna_low( void );

//! set the rfm70 lna gain to high
//
//! \ingroup highlevel
//! This sets the LNA gain of the receiver to the so-called
//! 'high' level. (I would have called it the 'normal' level.)
void lna_high( void );

//! set the rfm70 channel frequency
//
//! \ingroup highlevel
//! This method sets the frequency (channel) used by the rfm70 for 
//! receiving and transmitting to ( 2400 + ch ) MHz.
//! The highest bit of val is ignored, so the frequency range is
//! 2.4 .. 2.517 GHz. 
//! Not all of these frequencies might be free to use in your jurisdiction.
void channel( unsigned char ch );

//! set the rfm70 air data rate (baudrate)
//
//! \ingroup highlevel
//! This method sets the air data rate used by the rfm70 for 
//! receiving and transmitting. 
//! Allowed values are 1 and 2 (Mbits).
//! A value of 0 will have the same effect as a avlaue of 1.
//! A value > 2 will have the same effect as a value of 2.
//! Note that this is the bitrate the rfm70 uses in the 
//! packages that it sends. Due to various overhead factors
//! the data rate that a user of the module can achieve is much lower,
//! probably by a factor of 4.
void air_data_rate( unsigned char rate );

//! set the rfm70 CRC length
//
//! \ingroup highlevel
//! This method sets the length of the CRC used by the rfm70 in bytes.
//! Valid values are 0 (no CRC), 1 and 2.
//! A value > 2 has the same effect as the value 2.
//! Disabling the CRC disables the auto-acknowledge function.
//! Enabling the CRC does not automatically enable the
//! auto-acknowledge function.
void crc_length( unsigned char len );

//! set the rfm70 address length
//
//! \ingroup highlevel
//! This method sets the length (in bytes) of the addresses used by 
//! the rfm70. Valid values are 3, 4 and 5.
//! A value < 3 has the same effect as the value 3.
//! A value > 5 has the same effect as the value 5.
//! This setting is common for all data pipes.
void address_length( unsigned char len );

//! set the rfm70 transmit power
//
//! \ingroup highlevel
//! This method sets the rfm70 transmitter output power level.
//! - level == 0 => -10 dBm
//! - level == 1 =>  -5 dBm
//! - level == 2 =>   0 dBm
//! - level == 3 =>  +5 dBm
//! 
//! A level > 3 has the same effect as level == 3.
void power( unsigned char level ); 

//! set the retransmission delay and number of attempts
//
//! \ingroup highlevel
//! This method sets the delay d between retransmission attempts,
//! and the maximum number of attempts n.
//! The range of both arguments is 0..15. 
//! A value > 15 has the same effect as the value 15.
//!
//! The retransmission delay d is specified in steps of 250 us
//! with a minimum of 250 us:
//! - d == 0 => 250 us
//! - d == 1 => 500 us
//! - ...
//! - d == 15 => 4 ms
//!
//! The number of retransmissions n can range from
//! 0 (effectively disabling auto-retransmission) to 15.
void retransmit_delay_attempts( unsigned char d, unsigned char n );

//! read rfm70 retransmit count
//
//! \ingroup highlevel
//! This method reads and reports the number of retransmissions 
//! for the last packet that was sent. The number of retransmissions
//! is reset to zero when a new packet is sent.
unsigned char retransmit_count( void );

//! read rfm70 lost packets count
//
//! \ingroup highlevel
//! This method reads and reports the number of lost packets.
//! The range of this count is 0..15, at 15 it will not
//! increment when a next packet is lost.
//! The lost packets count is reset implicitly when the channel
//! is written (by calling channel() ) or 
//! explicitly by calling lost_packets_reset().
unsigned char lost_packets_count( void );

//! reset rfm70 lost packets count
//
//! \ingroup highlevel
//! This method resets the number of lost packets by reading and
//! re-writing the RF channel (RFM70_REG_RF_CH register).
void lost_packets_reset( void );

//! enables or disables the autoack on a pipe
//
//! \ingroup highlevel
//! This method enables or disables the auto acknowledgement
//! function on the specified pipe.
//!
//! pipe must be in the range 0..5. 
//! A pipe > 5 has the same effect as using pipe 5. 
void pipe_autoack( unsigned char pipe, bool enabled );

//! enables or disables a pipe
//
//! \ingroup highlevel
//! This method enables or disables the specified pipe.
//!
//! pipe must be in the range 0..5. 
//! A pipe > 5 has the same effect as using pipe 5. 
void pipe_enable( unsigned char d, bool enabled );

//! set the rfm70 pipe 0 address
//
//! \ingroup highlevel
//! This method sets the (up to 5 byte) receive address of pipe 0.
//! When the address length is set to less than 5 the excess
//! bytes are ignored, but address must still be a 5 byte array.
//!
//! Pipes 0 and 1 have a full (up to 5) byte address.
//! The other pipes (2..5) copy all but the least significant
//! (= first) byte from the pipe 1 address.
void receive_address_p0( const unsigned char address[ 5 ] );

//! set the rfm70 pipe 1 address
//
//! \ingroup highlevel
//! This method sets the (up to 5 byte) receive address of pipe 1.
//! When the address length is set to less than 5 the excess
//! bytes are ignored, but address must still be a 5 byte array.
//!
//! Pipes 0 and 1 have a full (up to 5) byte address.
//! The other pipes (2..5) copy all but the least significant
//! (= first) byte from the pipe 1 address.
void receive_address_p1( const unsigned char address[ 5 ] );

//! set the rfm70 pipe n (2..5) address
//
//! \ingroup highlevel
//! This method sets the least significant byte of 
//! the receive address of the pipe n.
//! The other bytes of the address are copied from pipe 1.
void receive_address_pn( unsigned char channel, unsigned char address );

//! set the payload size for pipe n
//
//! \ingroup highlevel
//! This method sets the size (= number of bytes, can be 1..32) 
//! for packets to be received  on pipe n. 
//! This setting must be the same as on the tranmitter.
//! A size of 0 will enable dynamic length packets.
//! A size > 32 will have the same effect as a size of 32.
void channel_payload_size( unsigned char n, unsigned char size );

//! set the rfm70 transmit address
//
//! \ingroup highlevel
//! This method sets the (up to 5 byte) address used
//! for all transmissions.
void transmit_address( const unsigned char *address );   

//! report whether the transmit fifo is full
//
//! \ingroup highlevel
//! This method reads and reports whether the transmit fifo is full.
bool transmit_fifo_full( void );

//! report whether the receive fifo is empty
//
//! \ingroup highlevel
//! This method reads and reports whether the receive fifo is full.
bool receive_fifo_empty( void );

//! transmit a message
//
//! \ingroup highlevel
//! This method transmits the specified message.
//!
//! The specified length must be less than or equal to 
//! RFM70_MAX_PACKET_LEN (32).
//! Specifying a larger length has the same effect as
//! specifying a length of RFM70_MAX_PACKET_LEN.
//!
//! The retransmission setting (set by
//! the method rfm70_retransmit_delay_attempts) determines
//! whether the message is transmitted on the air just once
//! or repeatedly until an acknowledge is received. 
//! 
//! The RFM70 must be in transmit mode.
void transmit_message(
   const unsigned char *buf,
   unsigned char length
);

//! transmit a message once
//
//! \ingroup highlevel
//! This method transmits the specified message once.
//!
//! The specified length must be less than or equal to 
//! RFM70_MAX_PACKET_LEN (32).
//! Specifying a larger length has the same effect as
//! specifying a length of RFM70_MAX_PACKET_LEN.
//!
//! The message is transmitted on the air once, irrespective
//! of the retransmission setting. 
//! 
//! The RFM70 must be in transmit mode.
void transmit_message_once(
   const unsigned char *buf,
   unsigned char length
);

//! get pipe number of the next message in receive FIFO
//
//! \ingroup highlevel
//! This method returns the number of the
//! pipe (0..5) on which the head message in the receive FIFO
//! was received.
//!
//! When the receive fifo is empty 0x07 is returned.
unsigned char receive_next_pipe( void );

//! get payload length of the next message in receive FIFO
//
//! \ingroup highlevel
//! This method returns length of the head message 
//! in the receive FIFO in bytes (1..32).
//! 
//! The RFM70 datasheet does not specify the value that is 
//! returned when the receive FIFO is empty
unsigned char receive_next_length( void );

//! (try to) receive a message
//
//! \ingroup highlevel
//! This method tries to receive a message.
//!
//! This method tries to retrieve a received message
//! from the receive FIFO. When no message is available
//! this method returns false. When a message is avaible
//! it is retrieved. The data is put in the buffer buf,
//! the length is written to length, and the method 
//! returns true.
//!
//! The size of the buffer buf must be at least
//! RFM70_MAX_PACKET_LEN (32).
//! 
//! The RFM70 must be in transmit mode.
bool receive(
   unsigned char & pipe,
   unsigned char *buf,
   unsigned char & length
);


}; // class frm70


#endif
