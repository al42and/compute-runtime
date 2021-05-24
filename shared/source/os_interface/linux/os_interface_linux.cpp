/*
 * Copyright (C) 2020-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/execution_environment/execution_environment.h"
#include "shared/source/execution_environment/root_device_environment.h"
#include "shared/source/gmm_helper/gmm_lib.h"
#include "shared/source/os_interface/hw_info_config.h"
#include "shared/source/os_interface/linux/drm_memory_operations_handler.h"
#include "shared/source/os_interface/linux/drm_neo.h"
#include "shared/source/os_interface/linux/sys_calls.h"
#include "shared/source/os_interface/os_interface.h"

#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

namespace NEO {

bool OSInterface::osEnabled64kbPages = false;
bool OSInterface::newResourceImplicitFlush = true;
bool OSInterface::gpuIdleImplicitFlush = true;
bool OSInterface::requiresSupportForWddmTrimNotification = false;

bool OSInterface::isDebugAttachAvailable() const {
    if (driverModel) {
        return driverModel->as<Drm>()->isDebugAttachAvailable();
    }
    return false;
}

bool RootDeviceEnvironment::initOsInterface(std::unique_ptr<HwDeviceId> &&hwDeviceId, uint32_t rootDeviceIndex) {
    Drm *drm = Drm::create(std::unique_ptr<HwDeviceIdDrm>(hwDeviceId.release()->as<HwDeviceIdDrm>()), *this);
    if (!drm) {
        return false;
    }

    osInterface.reset(new OSInterface());
    osInterface->setDriverModel(std::unique_ptr<DriverModel>(drm));
    auto hardwareInfo = getMutableHardwareInfo();
    HwInfoConfig *hwConfig = HwInfoConfig::get(hardwareInfo->platform.eProductFamily);
    if (hwConfig->configureHwInfoDrm(hardwareInfo, hardwareInfo, osInterface.get())) {
        return false;
    }
    memoryOperationsInterface = DrmMemoryOperationsHandler::create(*drm, rootDeviceIndex);
    return true;
}

} // namespace NEO