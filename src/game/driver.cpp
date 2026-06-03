#include "game/driver.h"

driver::driver(motorst* mot, bike_metadata* meta, recorder* rec, player_keys* keys)
    : mot(mot),
      meta(meta),
      rec(rec),
      keys(keys) {}
