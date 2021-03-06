/*
 *  Copyright 2018 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _LIBDM_DMTARGET_H_
#define _LIBDM_DMTARGET_H_

#include <linux/dm-ioctl.h>
#include <stdint.h>

#include <string>

#include <android-base/logging.h>

namespace android {
namespace dm {

class DmTargetTypeInfo {
  public:
    DmTargetTypeInfo() : major_(0), minor_(0), patch_(0) {}
    DmTargetTypeInfo(const struct dm_target_versions* info)
        : name_(info->name),
          major_(info->version[0]),
          minor_(info->version[1]),
          patch_(info->version[2]) {}

    const std::string& name() const { return name_; }
    std::string version() const {
        return std::to_string(major_) + "." + std::to_string(minor_) + "." + std::to_string(patch_);
    }

  private:
    std::string name_;
    uint32_t major_;
    uint32_t minor_;
    uint32_t patch_;
};

class DmTarget {
  public:
    DmTarget(uint64_t start, uint64_t length) : start_(start), length_(length) {}

    virtual ~DmTarget() = default;

    // Returns name of the target.
    virtual std::string name() const = 0;

    // Return the first logical sector represented by this target.
    uint64_t start() const { return start_; }

    // Returns size in number of sectors when this target is part of
    // a DmTable, return 0 otherwise.
    uint64_t size() const { return length_; }

    // Function that converts this object to a string of arguments that can
    // be passed to the kernel for adding this target in a table. Each target (e.g. verity, linear)
    // must implement this, for it to be used on a device.
    std::string Serialize() const;

  protected:
    // Get the parameter string that is passed to the end of the dm_target_spec
    // for this target type.
    virtual std::string GetParameterString() const = 0;

  private:
    // logical sector number start and total length (in terms of 512-byte sectors) represented
    // by this target within a DmTable.
    uint64_t start_, length_;
};

class DmTargetZero final : public DmTarget {
  public:
    DmTargetZero(uint64_t start, uint64_t length) : DmTarget(start, length) {}

    std::string name() const override { return "zero"; }
    std::string GetParameterString() const override;
};

class DmTargetLinear final : public DmTarget {
  public:
    DmTargetLinear(uint64_t start, uint64_t length, const std::string& block_device,
                   uint64_t physical_sector)
        : DmTarget(start, length), block_device_(block_device), physical_sector_(physical_sector) {}

    std::string name() const override { return "linear"; }
    std::string GetParameterString() const override;

  private:
    std::string block_device_;
    uint64_t physical_sector_;
};

}  // namespace dm
}  // namespace android

#endif /* _LIBDM_DMTARGET_H_ */
