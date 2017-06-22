#include <Arduino.h>		// provides _VARIANT_ARDUINO_DUE_X_ on Due

#if defined(__AVR_ATmega2560__)
#define	I_AM_MEGA
#endif

#if defined(_VARIANT_ARDUINO_DUE_X_)
#define	I_AM_DUE
#endif

#if defined(I_AM_MEGA) && defined(I_AM_DUE)
#error both a mega and a due
#endif

#if !defined(I_AM_MEGA) && !defined(I_AM_DUE)
#error not a mega and not a due
#endif

#ifdef I_AM_MEGA
// This is the EEPROM address where the unique node id is stored.
// The brain maps this node id to one of the six node
// numbers so it can adapt to hardware substitutions.
#define	TJP_NODE_ID	36	// arbitrary, The Journey has 36 segments
#endif
