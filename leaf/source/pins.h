#ifndef _PINS_H_
#define _PINS_H_

#include "RfMeshNodeConfig.h"

namespace pins {

	//! abstraction of a pin that can be used as digital output
	class output_pin {
		public:
			//! set the current value of a pin configured as output
			virtual void set( bool x )=0;
			//! configure the pin as output
			virtual void direction_set_output(){}
	};

	//! abstraction of a pin that can be used as digital input
	class input_pin {
		public:
			//! get the current value of a pin configured as input
			virtual bool get()=0;
			//! configure the pin as input
			virtual void direction_set_input(){}
	};

	enum direction { input, output };

	//! abstraction of a pin that can be used as digital input or output
	class input_output_pin : public output_pin, public input_pin {
		public:
			//! configure the pin as input or output
			virtual void direction_set( direction d ){
				if( d == input ){
					direction_set_input();
				} else {
					direction_set_output();
				}
			}

			//! configure the pin as input
			void direction_set_input(){
				direction_set( input );
			}

			//! configure the pin as output
			void direction_set_output(){
				direction_set( output );
			}
	};

	static void uc_init() {
#ifdef RF_MESH_HARDWARE_RASPBERRYPI
		wiringPiSetup();
#endif
	}

// direct port access on avr instead of using arduino 
// improves performance but increases code size as long
// as arduino functions are still used somewhere else
#ifdef RF_MESH_HARDWARE_RASPBERRYPI
	class uc_pin : public pins::input_output_pin {
		private:
			unsigned char nr;
		public:
			//! create a pin, specify the pin number
			uc_pin( unsigned char nr ): nr( nr ){}

			//! configure the pin as an output
			void direction_set_output(){
				pinMode( nr, OUTPUT );
			}

			//! configure the pin as an input
			void direction_set_input(){
				pinMode( nr, INPUT );
			}

			//! get the current value of a pin configured as input
			bool get(){
				return digitalRead( nr );
			}

			//! set the value of a pin configured as output
			void set( bool x ){
				digitalWrite( nr, x );
			}
	};
#endif

#ifdef RF_MESH_HARDWARE_ATMEGA328
   //! arduino pin that cab be used as input or output
	class uc_pin : public pins::input_output_pin {
   private:
      unsigned char nr;
   public:
      //! create a pin, specify the pin number
      uc_pin( unsigned char nr ): nr( nr ){}

      //! configure the pin as an output
      void direction_set_output(){
		 RFM70_DDR |= (1 << (nr - RFM70_BIT_OFFSET));
      }

      //! configure the pin as an input
      void direction_set_input(){
		 RFM70_DDR &= ~(1 << (nr - RFM70_BIT_OFFSET));
      }

      //! get the current value of a pin configured as input
      bool get(){
         return (RFM70_PIN & (1 << (nr - RFM70_BIT_OFFSET))) > 0;
      }

      //! set the value of a pin configured as output
      void set( bool x ){
		 if (x)
			RFM70_PORT |= (1 << (nr - RFM70_BIT_OFFSET));
		 else
			RFM70_PORT &= ~(1 << (nr - RFM70_BIT_OFFSET));
      }
   };
#endif

};

#endif
