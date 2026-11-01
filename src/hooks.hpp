#pragma once
#include "mods/api.h"

namespace uwunglish::hooks {
/**
 * Initialize the hooks that this mod uses
 *
 * @return `MOD_OK` or a specific `ModResult` value if failing
 */
ModResult initialize();

/**
 * Uninstall the hooks used by this mod
 *
 * @return `MOD_OK` or a specific `ModResult` value if failing
 */
ModResult uninstall();
}