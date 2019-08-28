/*
********************************************************************
*                                                                  *
* lwmf - lightweight media framework                               *
*                                                                  *
* (C) 2019 - present by Stefan Kubsch                              *
*                                                                  *
********************************************************************
*/

#pragma once

#define WIN32_LEAN_AND_MEAN

// *************************************************
// ** YOU CAN MAKE SOME SETTINGS HERE !!!         **
// *************************************************

// Set LoggingEnabled to "false" if you don´t want to write any logsfiles!
constexpr bool LoggingEnabled{ true };
// Set ThrowExceptions to "false" if you don´t want to handle errors by exceptions!
constexpr bool ThrowExceptions{ true };

#include "lwmf_logging.hpp"

// Establish lwmf system logfile
// Will not be created if "LoggingEnabled" = false !
inline lwmf::Logging LWMFSystemLog("lwmf_systemlog.log");

#include "lwmf_simd.hpp"
#include "lwmf_math.hpp"
#include "lwmf_general.hpp"
#include "lwmf_color.hpp"
#include "lwmf_openglloader.hpp"
#include "lwmf_openglwindow.hpp"
#include "lwmf_openglshader.hpp"
#include "lwmf_rawinput.hpp"
#include "lwmf_primitives.hpp"
#include "lwmf_texture.hpp"
#include "lwmf_bmp.hpp"
#include "lwmf_png.hpp"
#include "lwmf_text.hpp"
#include "lwmf_mp3.hpp"
#include "lwmf_gamepad.hpp"
#include "lwmf_perlinnoise.hpp"
#include "lwmf_fpscounter.hpp"
#include "lwmf_multithreading.hpp"
#include "lwmf_inifile.hpp"