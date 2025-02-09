/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libfoundation/EventReceiver.h>

namespace LFoundation {

class Object : public LFoundation::EventReceiver {
public:
    Object() = default;
    ~Object() = default;
};

} // namespace LFoundation