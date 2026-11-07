#pragma once
#include "mods/api.h"

namespace uwunglish::hooks {
/**
 * Initialize the hooks that this mod uses
 *
 * @return `MOD_OK` or a specific `ModResult` value if failing
 */
// TODO, this is not a clean way to send this info, should still have no parameter and call the
// util.hpp instead
ModResult initialize(bool);

/**
 * Uninstall the hooks used by this mod
 *
 * @return `MOD_OK` or a specific `ModResult` value if failing
 */
ModResult uninstall();
}