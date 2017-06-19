#include <FastLED.h>

#define NUM_NODES 3
#define RINGS_PER_NODE 12
#define STRIPS_PER_NODE 4
#define PHYSICAL_LEDS_PER_RING 420
#define LEDS_PER_RING 408

#define LEDS_PER_STRIP (2*PHYSICAL_LEDS_PER_RING + LEDS_PER_RING + 1)
#define PHYSICAL_LEDS_PER_NODE (LEDS_PER_STRIP*STRIPS_PER_NODE)

CRGB leds_raw[PHYSICAL_LEDS_PER_NODE];
CRGBSet leds[RINGS_PER_NODE] = {
  CRGBSet(&leds_raw[1], LEDS_PER_RING),
  CRGBSet(&leds_raw[1 +                      PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[1 +                    2*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[2 +   LEDS_PER_RING +  2*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[2 +   LEDS_PER_RING +  3*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[2 +   LEDS_PER_RING +  4*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[3 + 2*LEDS_PER_RING +  4*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[3 + 2*LEDS_PER_RING +  5*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[3 + 2*LEDS_PER_RING +  6*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[4 + 3*LEDS_PER_RING +  6*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[4 + 3*LEDS_PER_RING +  7*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING),
  CRGBSet(&leds_raw[4 + 3*LEDS_PER_RING +  8*PHYSICAL_LEDS_PER_RING], LEDS_PER_RING)
};

uint8_t node_number = 0;
unsigned long loop_count = 0;

void setup() {
  Serial.begin(115200);
  LEDS.addLeds<WS2811_PORTD, 8>(leds_raw, LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  LEDS.clear();
}

void loop() {
  uint8_t ringOffset = node_number < NUM_NODES ? node_number*RINGS_PER_NODE : 0;
  uint8_t unlitPixels = node_number < NUM_NODES ? node_number + 1 : 8;

  for(uint8_t ring = ringOffset; ring < ringOffset + RINGS_PER_NODE; ring++)
  {
    // Clear pixels
    leds[ring] = CRGB::Black;

    // Pick color based on ring
    uint8_t temp = ring % 4;
    CRGB ringColor = temp == 0 ? CRGB::Red : temp == 1 ? CRGB::Green : temp == 2 ? CRGB::Blue : CRGB(120, 120, 120);

    // Determine strand number and number of pixels to light up
    temp = (ring - ringOffset) / 3; // Strand #
    uint8_t litPixels = 1 + temp;

    // Determine period of repeating pattern and get an extended LED count that is a multiple of the period
    uint8_t period = litPixels + unlitPixels;
    uint16_t extendedLEDCount = ((LEDS_PER_RING-1)/period+1)*period;

    // Loop over extended LED count, but don't write the ones that are outside the range of the physical LEDs
    for(uint16_t curPixel = 0; curPixel < extendedLEDCount; curPixel++) {
      uint16_t idx = (curPixel + loop_count/10) % extendedLEDCount; // Add scrolling based off loop_count
      if(idx >= LEDS_PER_RING) continue;

      if(curPixel % period < litPixels) {
        leds[ring][idx] = ringColor;
      }
    }
  }

  LEDS.show();
  loop_count++;
}
