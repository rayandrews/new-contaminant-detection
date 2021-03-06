#ifndef LIB_STORAGE_STORAGE_HPP_
#define LIB_STORAGE_STORAGE_HPP_

/** @file storage.hpp
 *  @brief Precompiled header for faster project compilation
 *
 * Every source file must include this file whether precompiled feature is
 * enabled or not
 */
#pragma GCC system_header

// 1. STL
#include <condition_variable>
#include <cstdint>
#include <string>

// 2. Vendor
#include <sqlite3.h>
#include <sqlite_orm/sqlite_orm.h>

// 3. Internal Project
#include <libutil/util.hpp>

#include <libcore/core.hpp>

#include <libstorage/storage.hpp>

#include <libgui/gui.hpp>

// 4. Local
#include "schema.hpp"

#include "database.hpp"

#include "listener.hpp"

#endif  // LIB_STORAGE_STORAGE_HPP_
