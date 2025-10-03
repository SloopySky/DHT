# DHT
DHT temperature and humidity sensor library

## Datasheet:
```https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf?srsltid=AfmBOopHotJ2sH__bwBmUq2g-vpm-BXT5C-LWXV-AjG1lcc_4owfIj3_```

# Usage
This DHT library is platfrom independent.

It calls external interfaces that must be implemented in the platform code.

Refer to 'examples' folder.

In ESP32 case, DHT is a submodule nested inside an idf component, that implements platform dependencies.

# Build

## Platform stubs
Set ```DHT_STUBS_ENABLE``` variable

## Test build
Run cmake with option ```-DDHT_TEST_ENABLE=ON```

# Running tests
Run ```DhtTest``` executable
