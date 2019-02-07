/**
 *  Copyright (c) 2019 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */

#pragma once

#include <PrimaryKeyEngine.h>
#include <daqdb/Key.h>
#include <daqdb/Options.h>

namespace DaqDB {

class PrimaryKeyBase : public DaqDB::PrimaryKeyEngine {
  public:
    PrimaryKeyBase(const DaqDB::Options &options);
    virtual ~PrimaryKeyBase();
    void dequeueNext(Key &key);
    void enqueueNext(const Key &key);
    bool isLocal(const Key &key);

  protected:
    size_t _keySize;
    size_t _pKeySize;
    size_t _pKeyOffset;

  private:
    uint32_t _localKeyMask;
    uint32_t _localKeyValue;
};

} // namespace DaqDB
