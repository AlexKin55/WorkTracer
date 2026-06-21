#pragma once

#include <qmetatype.h>
#include "IActivityWatcher.hpp"
#include "IScreenshot.hpp"

#include <memory>

std::unique_ptr<IActivityWatcher> createActivityWatcher(int timeoutSec, Display* x11Display = nullptr) ;
std::unique_ptr<IScreenshot> createScreenShotEngine();
