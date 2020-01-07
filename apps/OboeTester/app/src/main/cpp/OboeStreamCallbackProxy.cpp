/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common/OboeDebug.h"
#include "OboeStreamCallbackProxy.h"

// Linear congruential random number generator.
static uint32_t s_random16() {
    static uint32_t seed = 1234;
    seed = ((seed * 31421) + 6927) & 0x0FFFF;
    return seed;
}

/**
 * The random number generator is good for burning CPU because the compiler cannot
 * easily optimize away the computation.
 * @param workload number of times to execute the loop
 * @return a white noise value between -1.0 and +1.0
 */
static float s_burnCPU(int32_t workload) {
    uint32_t random = 0;
    for (int32_t i = 0; i < workload; i++) {
        for (int32_t j = 0; j < 10; j++) {
            random = random ^ s_random16();
        }
    }
    return (random - 32768) * (1.0 / 32768);
}

bool OboeStreamCallbackProxy::mCallbackReturnStop = false;

oboe::DataCallbackResult OboeStreamCallbackProxy::onAudioReady(
        oboe::AudioStream *audioStream,
        void *audioData,
        int numFrames) {
    mCallbackCount++;
    if (mCallbackReturnStop) {
        return oboe::DataCallbackResult::Stop;
    }

    //
    s_burnCPU((int32_t)(mWorkload * kWorkloadScaler * numFrames));

    if (mCallback != nullptr) {
        return mCallback->onAudioReady(audioStream, audioData, numFrames);
    }
    return oboe::DataCallbackResult::Stop;
}

void OboeStreamCallbackProxy::onErrorBeforeClose(oboe::AudioStream *audioStream, oboe::Result error) {
    LOGD("OboeStreamCallbackProxy::%s(%p, %d) called", __func__, audioStream, error);
    if (mCallback != nullptr) {
        mCallback->onErrorBeforeClose(audioStream, error);
    }
}

void OboeStreamCallbackProxy::onErrorAfterClose(oboe::AudioStream *audioStream, oboe::Result  error) {

    LOGD("OboeStreamCallbackProxy::%s(%p, %d) called", __func__, audioStream, error);
    if (mCallback != nullptr) {
        mCallback->onErrorAfterClose(audioStream, error);
    }
}
